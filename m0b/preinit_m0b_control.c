// hmos2negroni preinit  (M0b first stage /init)
//
// Runs as PID 1 from the HarmonyOS ramdisk (taken from the Pura 70 OTA) with
// the phone's 5.10 GKI kernel.  It does what hmos2sweet's preinit does, adapted
// to the OnePlus 10 Pro:
//   * the kernel has no CONFIG_DEVTMPFS -> mini-ueventd builds /dev and
//     /dev/block/by-name from /sys/class/block/*/uevent
//   * super LP metadata -> one dm-linear device per logical partition
//   * /etc/fstab.required for the erofs partitions (read by HarmonyOS init)
//   * logs to /dev/kmsg (-> ramoops/pstore) and, if present, to cache
//   * execs HarmonyOS /bin/init_early
//
// Build: aarch64-linux-gnu-gcc -O2 -static -o preinit preinit.c

#define _GNU_SOURCE
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <linux/dm-ioctl.h>
#include <linux/fs.h>
#include <linux/reboot.h>
#include <sys/reboot.h>

#define REAL_INIT "/system/bin/init"
#define LP_RESERVED 4096
#define LP_GEOMETRY_MAGIC 0x616c4467u
#define LP_HEADER_MAGIC 0x414C5030u
#define LP_ATTR_READONLY 1u

static int kfd = -1;
static int dm_ctl = -1;
static FILE *fstab;
static FILE *maplog;
static int cache_fd = -1;
static char cache_buf[1 << 20];
static size_t cache_len;

/* logical partition -> HarmonyOS mount point (first stage, read-only) */
static const struct { const char *name, *mnt, *flags; } hm_mounts[] = {
	{ "system",    "/usr",       "wait,required" },
	{ "vendor",    "/vendor",    "wait,required" },
	{ "chip_prod", "/chip_prod", "wait,required,nofail" },
	{ "sys_prod",  "/sys_prod",  "wait,required,nofail" },
	{ "cust",      "/cust",      "wait,required,nofail" },
	{ "patch",     "/patch_hw",  "wait,required,nofail" },
	{ "version",   "/version",   "wait,required,nofail" },
	{ "preload",   "/preload",   "wait,required,nofail" },
};

static void klog(const char *fmt, ...)
{
	char buf[1024];
	int n = snprintf(buf, sizeof(buf), "<3>hmos-preinit: ");
	va_list ap;

	va_start(ap, fmt);
	n += vsnprintf(buf + n, sizeof(buf) - n, fmt, ap);
	va_end(ap);
	if (n > (int)sizeof(buf) - 2)
		n = sizeof(buf) - 2;
	buf[n++] = '\n';
	if (kfd >= 0 && write(kfd, buf, n) < 0)
		; /* nowhere to report this */
	if (cache_fd >= 0 && cache_len + (size_t)n < sizeof(cache_buf)) {
		memcpy(cache_buf + cache_len, buf, n);
		cache_len += n;
	}
}

/* ---------- first-stage kernel modules (ufs_qcom etc. are modules) ---------- */

/* --- raw log partition writer ---------------------------------------- */
static int rlog_fd = -1;
static int rlog_off;

static int part_lookup(const char *want, char *out, size_t len)
{
	DIR *d = opendir("/sys/class/block");
	struct dirent *e;

	if (!d)
		return -1;
	while ((e = readdir(d))) {
		char p[300], buf[1024];
		int fd;
		ssize_t n;

		if (e->d_name[0] == '.')
			continue;
		snprintf(p, sizeof(p), "/sys/class/block/%s/uevent", e->d_name);
		fd = open(p, O_RDONLY);
		if (fd < 0)
			continue;
		n = read(fd, buf, sizeof(buf) - 1);
		close(fd);
		if (n <= 0)
			continue;
		buf[n] = 0;
		if (strstr(buf, want)) {
			snprintf(out, len, "/dev/block/%s", e->d_name);
			closedir(d);
			return 0;
		}
	}
	closedir(d);
	return -1;
}

static void rlog_init(void)
{
	char dev[96];

	if (rlog_fd >= 0)
		return;
	if (part_lookup("PARTNAME=logdump", dev, sizeof(dev)) != 0)
		return;
	rlog_fd = open(dev, O_RDWR);
	if (rlog_fd >= 0)
		rlog_off = 0;
}

static void rlog(const char *fmt, ...)
{
	char buf[240];
	va_list ap;
	int n;

	if (rlog_fd < 0)
		rlog_init();
	if (rlog_fd < 0)
		return;
	va_start(ap, fmt);
	n = vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	if (n < 0)
		return;
	if (rlog_off + 256 > 65536)
		return;
	{
		char slot[256];
		memset(slot, 0, sizeof(slot));
		if (n > 255)
			n = 255;
		memcpy(slot, buf, n);
		buf[n] = 0;
		(void)!pwrite(rlog_fd, slot, sizeof(slot), 4096 + rlog_off);
	}
	rlog_off += 256;
	fsync(rlog_fd);
}

static int n_ok, n_fail;

static void fail_stage(int stage)
{
	klog("FAIL at stage %d -> power off in %ds", stage, 10 + stage);
	sync();
	sleep(10 + stage);
	reboot(RB_POWER_OFF);
	for (;;)
		pause();
}

/* Dependency closure of ufs_qcom.ko, taken from the OEM modules.dep/softdep.
 * Note the oplus modules: ufs_qcom will not load without them.
 */
static const char *mod_order[] = {
	"tmecom-intf.ko", "hwkm.ko", "qcom-scm.ko", "crypto-qti-hwkm.ko",
	"crypto-qti-common.ko", "ufshcd-crypto-qti.ko", "smem.ko", "minidump.ko",
	"qcom_ipc_logging.ko", "proxy-consumer.ko", "debug-regulator.ko",
	"gdsc-regulator.ko", "clk-qcom.ko", "phy-qcom-ufs.ko", "simcardnum.ko",
	"buildvariant.ko", "oplusboot.ko", "oplus_project.ko", "device_info.ko",
	"ufs_qcom.ko", "clk-dummy.ko", "gcc-waipio.ko", "cmd-db.ko",
	"qcom_rpmh.ko", "clk-rpmh.ko", "icc-debug.ko", "icc-bcm-voter.ko",
	"icc-rpmh.ko", "phy-qcom-ufs-qmp-v4-waipio.ko",
};

static void load_modules(void)
{
	FILE *f;
	char line[256];
	char names[512][96];
	int n = 0, i, pass;
	int blocked[512];
	int loaded[512];
	int ok, fail;

	f = fopen("/lib/modules/modules.load", "r");
	if (!f) {
		klog("modules.load missing");
		n_ok = 0; n_fail = -1;
		return;
	}
	memset(names, 0, sizeof(names));
	while (fgets(line, sizeof(line), f) && n < 512) {
		line[strcspn(line, "\r\n")] = 0;
		if (!line[0] || line[0] == '#')
			continue;
		snprintf(names[n], sizeof(names[n]), "%s", line);
		n++;
	}
	fclose(f);

	/* blocklist, matched after the names are known */
	memset(blocked, 0, sizeof(blocked));
	f = fopen("/lib/modules/modules.blocklist", "r");
	if (f) {
		char b[96];
		while (fgets(b, sizeof(b), f)) {
			b[strcspn(b, "\r\n")] = 0;
			if (!b[0] || b[0] == '#')
				continue;
			for (i = 0; i < n; i++)
				if (!strcmp(names[i], b))
					blocked[i] = 1;
		}
		fclose(f);
	}

	memset(loaded, 0, sizeof(loaded));
	for (pass = 0; pass < 8; pass++) {
		int progress = 0;
		for (i = 0; i < n; i++) {
			char path[320];
			int fd;
			if (loaded[i] || blocked[i])
				continue;
			snprintf(path, sizeof(path), "/lib/modules/%s", names[i]);
			fd = open(path, O_RDONLY | O_CLOEXEC);
			if (fd < 0) { loaded[i] = 1; continue; }
			if (syscall(SYS_finit_module, fd, "", 0) == 0 || errno == EEXIST) {
				loaded[i] = 1;
				progress = 1;
			}
			close(fd);
		}
		if (!progress)
			break;
	}
	ok = fail = 0;
	for (i = 0; i < n; i++) {
		if (loaded[i]) ok++; else fail++;
	}
	klog("modules: %d entries, ok=%d fail=%d (multi-pass)", n, ok, fail);
	n_ok = ok;
	n_fail = fail;
}

static int read_file(const char *path, char *buf, size_t len)
{
	int fd = open(path, O_RDONLY);
	ssize_t n;

	if (fd < 0)
		return -1;
	n = read(fd, buf, len - 1);
	close(fd);
	if (n < 0)
		return -1;
	buf[n] = 0;
	return 0;
}

static void mkdir_p(const char *path)
{
	char tmp[256];
	size_t i;

	snprintf(tmp, sizeof(tmp), "%s", path);
	for (i = 1; tmp[i]; i++) {
		if (tmp[i] == '/') {
			tmp[i] = 0;
			mkdir(tmp, 0755);
			tmp[i] = '/';
		}
	}
	mkdir(tmp, 0755);
}

static int mknod_dev(const char *path, int major, int minor, mode_t mode)
{
	if (mknod(path, mode, makedev(major, minor)) == 0)
		return 0;
	return errno == EEXIST ? 0 : -1;
}

/* ------------------------------------------------------------ mini ueventd */
static void uevent_block(void)
{
	DIR *d = opendir("/sys/class/block");
	struct dirent *de;
	int nodes = 0, links = 0;

	if (!d) {
		klog("ueventd: no /sys/class/block: %s", strerror(errno));
		return;
	}
	mkdir_p("/dev/block/by-name");
	while ((de = readdir(d))) {
		char uevent[512], line[1024], devname[256] = "", partname[64] = "";
		int maj = -1, min = -1;
		FILE *f;

		if (de->d_name[0] == '.')
			continue;
		snprintf(uevent, sizeof(uevent), "/sys/class/block/%s/uevent", de->d_name);
		f = fopen(uevent, "r");
		if (!f)
			continue;
		while (fgets(line, sizeof(line), f)) {
			line[strcspn(line, "\n")] = 0;
			if (!strncmp(line, "MAJOR=", 6))
				maj = atoi(line + 6);
			else if (!strncmp(line, "MINOR=", 6))
				min = atoi(line + 6);
			else if (!strncmp(line, "DEVNAME=", 8))
				snprintf(devname, sizeof(devname), "%s", line + 8);
			else if (!strncmp(line, "PARTNAME=", 9))
				snprintf(partname, sizeof(partname), "%s", line + 9);
		}
		fclose(f);
		if (maj < 0 || min < 0)
			continue;
		if (!devname[0])
			snprintf(devname, sizeof(devname), "%s", de->d_name);
		char node[512], link[512], tgt[256];
		snprintf(node, sizeof(node), "/dev/block/%s", devname);
		if (mknod_dev(node, maj, min, S_IFBLK | 0600) == 0)
			nodes++;
		if (partname[0]) {
			snprintf(link, sizeof(link), "/dev/block/by-name/%s", partname);
			unlink(link);
			snprintf(tgt, sizeof(tgt), "../%s", devname);
			if (symlink(tgt, link) == 0)
				links++;
		}
	}
	closedir(d);
	klog("ueventd: %d block nodes, %d by-name links", nodes, links);
}

static void uevent_class(const char *cls, const char *devsub)
{
	DIR *d = opendir(cls);
	struct dirent *de;
	char dir[128];
	int n = 0;

	if (!d)
		return;
	snprintf(dir, sizeof(dir), "/dev/%s", devsub);
	mkdir_p(dir);
	while ((de = readdir(d))) {
		char syspath[512], buf[64], node[512];
		int maj, min;

		if (de->d_name[0] == '.')
			continue;
		snprintf(syspath, sizeof(syspath), "%s/%s/dev", cls, de->d_name);
		if (read_file(syspath, buf, sizeof(buf)) != 0)
			continue;
		if (sscanf(buf, "%d:%d", &maj, &min) != 2)
			continue;
		snprintf(node, sizeof(node), "%s/%s", dir, de->d_name);
		if (mknod_dev(node, maj, min, S_IFCHR | 0600) == 0)
			n++;
	}
	closedir(d);
	if (n)
		klog("ueventd: %d nodes under /dev/%s", n, devsub);
}

static void uevent_all(void)
{
	mknod_dev("/dev/null", 1, 3, S_IFCHR | 0666);
	mknod_dev("/dev/zero", 1, 5, S_IFCHR | 0666);
	mknod_dev("/dev/full", 1, 7, S_IFCHR | 0666);
	mknod_dev("/dev/random", 1, 8, S_IFCHR | 0666);
	mknod_dev("/dev/urandom", 1, 9, S_IFCHR | 0666);
	mknod_dev("/dev/console", 5, 1, S_IFCHR | 0600);
	mknod_dev("/dev/tty", 5, 0, S_IFCHR | 0666);
	/* locate the real watchdog device instead of guessing its minor */
	{
		DIR *d = opendir("/sys/class/watchdog");
		struct dirent *e;

		if (d) {
			while ((e = readdir(d))) {
				char p[300], buf[64];
				int fd, maj, min;

				if (e->d_name[0] == '.')
					continue;
				snprintf(p, sizeof(p), "/sys/class/watchdog/%s/dev", e->d_name);
				fd = open(p, O_RDONLY);
				if (fd < 0)
					continue;
				memset(buf, 0, sizeof(buf));
				if (read(fd, buf, sizeof(buf) - 1) > 0 &&
				    sscanf(buf, "%d:%d", &maj, &min) == 2) {
					snprintf(p, sizeof(p), "/dev/%s", e->d_name);
					mknod_dev(p, maj, min, S_IFCHR | 0600);
					mknod_dev("/dev/watchdog", maj, min, S_IFCHR | 0600);
					klog("watchdog %s -> %d:%d", e->d_name, maj, min);
				}
				close(fd);
			}
			closedir(d);
		}
	}

	mknod_dev("/dev/kmsg", 1, 11, S_IFCHR | 0600);
	mknod_dev("/dev/device-mapper", 10, 236, S_IFCHR | 0600);
	uevent_block();
	uevent_class("/sys/class/input", "input");
	uevent_class("/sys/class/drm", "dri");
	uevent_class("/sys/class/misc", "");
	uevent_class("/sys/class/graphics", "graphics");
}

/* ------------------------------------------------------------------- super */
#pragma pack(push, 1)
struct lp_geometry {
	uint32_t magic, struct_size;
	uint8_t checksum[32];
	uint32_t metadata_max_size, metadata_slot_count, logical_block_size;
};
struct lp_table_desc { uint32_t offset, num_entries, entry_size; };
struct lp_header {
	uint32_t magic;
	uint16_t major, minor;
	uint32_t header_size;
	uint8_t header_checksum[32];
	uint32_t tables_size;
	uint8_t tables_checksum[32];
	struct lp_table_desc partitions, extents, groups, block_devices;
};
struct lp_partition {
	char name[36];
	uint32_t attributes, first_extent_index, num_extents, group_index;
};
struct lp_extent {
	uint64_t num_sectors;
	uint32_t target_type;
	uint64_t target_data;
	uint32_t target_source;
};
#pragma pack(pop)


static int block_count(void)
{
	DIR *d = opendir("/sys/class/block");
	struct dirent *e;
	int n = 0;

	if (!d)
		return 0;
	while ((e = readdir(d))) {
		char p[300];
		if (e->d_name[0] == '.')
			continue;
		snprintf(p, sizeof(p), "/sys/class/block/%s/dev", e->d_name);
		if (access(p, F_OK) == 0 && strncmp(e->d_name, "loop", 4) &&
		    strncmp(e->d_name, "ram", 3))
			n++;
	}
	closedir(d);
	return n;
}

static int part_dev(const char *want, char *out, size_t len)
{
	DIR *d = opendir("/sys/class/block");
	struct dirent *e;

	if (!d)
		return -1;
	while ((e = readdir(d))) {
		char p[300], buf[1024];
		int fd;
		ssize_t n;

		if (e->d_name[0] == '.')
			continue;
		snprintf(p, sizeof(p), "/sys/class/block/%s/uevent", e->d_name);
		fd = open(p, O_RDONLY);
		if (fd < 0)
			continue;
		n = read(fd, buf, sizeof(buf) - 1);
		close(fd);
		if (n <= 0)
			continue;
		buf[n] = 0;
		if (strstr(buf, want)) {
			snprintf(out, len, "/dev/block/%s", e->d_name);
			closedir(d);
			return 0;
		}
	}
	closedir(d);
	return -1;
}

static int find_super(char *devpath, size_t len)
{
	if (access("/dev/block/by-name/super", F_OK) == 0) {
		snprintf(devpath, len, "/dev/block/by-name/super");
		return 0;
	}
	DIR *d = opendir("/sys/class/block");
	struct dirent *e;

	if (!d)
		return -1;
	while ((e = readdir(d))) {
		char p[300], buf[1024];
		int fd;
		ssize_t n;

		if (e->d_name[0] == '.')
			continue;
		snprintf(p, sizeof(p), "/sys/class/block/%s/uevent", e->d_name);
		fd = open(p, O_RDONLY);
		if (fd < 0)
			continue;
		n = read(fd, buf, sizeof(buf) - 1);
		close(fd);
		if (n <= 0)
			continue;
		buf[n] = 0;
		if (strstr(buf, "PARTNAME=super\n")) {
			snprintf(devpath, len, "/dev/block/%s", e->d_name);
			closedir(d);
			return 0;
		}
	}
	closedir(d);
	return -1;
}

static void dm_init(struct dm_ioctl *io, size_t size, const char *name)
{
	memset(io, 0, size);
	io->version[0] = DM_VERSION_MAJOR;
	io->version[1] = 0;
	io->version[2] = 0;
	io->data_size = size;
	io->data_start = sizeof(*io);
	strncpy(io->name, name, sizeof(io->name) - 1);
}

static int dm_create_linear(const char *name, int ro, const char *super_mm,
			    struct lp_extent *ext, uint32_t n_ext, unsigned *minor_out)
{
	char buf[16384];
	struct dm_ioctl *io = (struct dm_ioctl *)buf;
	uint64_t start = 0;

	dm_init(io, sizeof(buf), name);
	if (ioctl(dm_ctl, DM_DEV_CREATE, io)) {
		klog("DM_DEV_CREATE %s: %s", name, strerror(errno));
		return -1;
	}
	*minor_out = minor(io->dev);

	dm_init(io, sizeof(buf), name);
	if (ro)
		io->flags |= DM_READONLY_FLAG;
	char *p = buf + sizeof(*io);
	char *end = buf + sizeof(buf);
	for (uint32_t i = 0; i < n_ext; i++) {
		struct dm_target_spec *ts = (struct dm_target_spec *)p;
		char *params = p + sizeof(*ts);
		memset(ts, 0, sizeof(*ts));
		ts->sector_start = start;
		ts->length = ext[i].num_sectors;
		if (ext[i].target_type == 0) {
			strcpy(ts->target_type, "linear");
			snprintf(params, end - params, "%s %llu", super_mm,
				 (unsigned long long)ext[i].target_data);
		} else {
			strcpy(ts->target_type, "zero");
			params[0] = 0;
		}
		size_t l = sizeof(*ts) + strlen(params) + 1;
		l = (l + 7) & ~(size_t)7;
		ts->next = l;
		p += l;
		start += ext[i].num_sectors;
		io->target_count++;
	}
	if (ioctl(dm_ctl, DM_TABLE_LOAD, io)) {
		klog("DM_TABLE_LOAD %s: %s", name, strerror(errno));
		return -1;
	}
	dm_init(io, sizeof(buf), name);
	if (ioctl(dm_ctl, DM_DEV_SUSPEND, io)) {
		klog("DM resume %s: %s", name, strerror(errno));
		return -1;
	}
	return 0;
}

static unsigned system_minor;
static int have_system;

static void fstab_add(const char *name, unsigned mn)
{
	if (!strcmp(name, "system")) {
		system_minor = mn;
		have_system = 1;
	}
	for (size_t i = 0; i < sizeof(hm_mounts) / sizeof(hm_mounts[0]); i++)
		if (!strcmp(name, hm_mounts[i].name) && fstab)
			fprintf(fstab, "/dev/block/dm-%u %s erofs ro %s\n", mn,
				hm_mounts[i].mnt, hm_mounts[i].flags);
}

static int map_super(void)
{
	char super[64];
	struct stat st;
	struct lp_geometry g;
	char super_mm[32];
	char *meta;
	struct lp_header *h;
	char *tables;
	off_t moff;
	int fd;

	if (find_super(super, sizeof(super))) {
		klog("super partition not found");
		return -1;
	}
	fd = open(super, O_RDONLY);
	if (fd < 0 || fstat(fd, &st)) {
		klog("open %s: %s", super, strerror(errno));
		return -1;
	}
	snprintf(super_mm, sizeof(super_mm), "%u:%u", major(st.st_rdev), minor(st.st_rdev));
	if (pread(fd, &g, sizeof(g), LP_RESERVED) != sizeof(g) || g.magic != LP_GEOMETRY_MAGIC) {
		klog("bad LP geometry on %s (magic 0x%08x)", super, g.magic);
		return -1;
	}
	/* primary metadata slot 0: reserved + geometry + backup geometry */
	moff = LP_RESERVED + 2 * 4096;
	meta = malloc(g.metadata_max_size);
	if (!meta || pread(fd, meta, g.metadata_max_size, moff) != (ssize_t)g.metadata_max_size) {
		klog("cannot read LP metadata");
		return -1;
	}
	h = (struct lp_header *)meta;
	if (h->magic != LP_HEADER_MAGIC) {
		klog("bad LP header magic 0x%08x", h->magic);
		return -1;
	}
	tables = meta + h->header_size;
	klog("super=%s (%s) LP v%u.%u, %u partitions, max_size %u", super, super_mm,
	     h->major, h->minor, h->partitions.num_entries, g.metadata_max_size);
	rlog("geo: slots=%u max_size=%u parts=%u hdr=%u", g.metadata_slot_count,
	     g.metadata_max_size, h->partitions.num_entries, h->header_size);

	dm_ctl = open("/dev/device-mapper", O_RDWR);
	if (dm_ctl < 0) {
		klog("no device-mapper: %s", strerror(errno));
		return -1;
	}
	mkdir_p("/etc");
	fstab = fopen("/etc/fstab.required", "w");
	if (fstab)
		fprintf(fstab, "# generated by hmos-preinit from the super LP metadata\n");
	maplog = fopen("/hmos_dm.txt", "w");

	for (uint32_t i = 0; i < h->partitions.num_entries; i++) {
		struct lp_partition *pt = (struct lp_partition *)(tables + h->partitions.offset +
								 i * h->partitions.entry_size);
		struct lp_extent ext[64];
		char name[37];
		uint32_t n;
		unsigned mn = 0;

		memcpy(name, pt->name, 36);
		name[36] = 0;
		if (!pt->num_extents)
			continue;
		n = pt->num_extents > 64 ? 64 : pt->num_extents;
		for (uint32_t k = 0; k < n; k++)
			memcpy(&ext[k], tables + h->extents.offset +
					    (pt->first_extent_index + k) * h->extents.entry_size,
			       sizeof(struct lp_extent));
		{
			int dmrc = dm_create_linear(name, pt->attributes & LP_ATTR_READONLY,
						    super_mm, ext, n, &mn);
			if (dmrc) {
				rlog("dm %s FAILED rc=%d (%u extents)", name, dmrc, n);
			} else {
				klog("mapped %s -> dm-%u (%u extents)", name, mn, n);
				rlog("dm %s -> dm-%u (%u ext)", name, mn, n);
				if (maplog)
					fprintf(maplog, "%s dm-%u\n", name, mn);
				fstab_add(name, mn);
			}
		}
	}
	if (maplog)
		fclose(maplog);
	if (fstab)
		fclose(fstab);
	free(meta);
	close(fd);
	return 0;
}

/* ------------------------------------------------------------------- cache */
static void cache_logger(void)
{
	struct stat st;

	{
		char cdev[96];
		if (part_dev("PARTNAME=cache", cdev, sizeof(cdev)) != 0)
			return;
		mkdir_p("/cache");
		if (mount(cdev, "/cache", "ext4", 0, NULL) &&
		    mount(cdev, "/cache", "f2fs", 0, NULL))
			return;
	}
	cache_fd = open("/cache/hmos-preinit.log", O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (cache_fd >= 0)
		klog("cache logger on /cache/hmos-preinit.log");
}

static void cache_flush(void)
{
	if (cache_fd >= 0 && cache_len) {
		if (write(cache_fd, cache_buf, cache_len) < 0)
			; /* nothing better to do */
		fsync(cache_fd);
		close(cache_fd);
		cache_fd = -1;
	}
}

int main(int argc, char **argv)
{
	char cmdline[4096] = "";
	(void)argc;
	(void)argv;

	mkdir_p("/proc");
	mkdir_p("/sys");
	mkdir_p("/dev");
	mkdir_p("/tmp");
	mount("proc", "/proc", "proc", 0, NULL);
	mount("sysfs", "/sys", "sysfs", 0, NULL);
	mount("tmpfs", "/dev", "tmpfs", 0, "mode=0755");
	mount("tmpfs", "/tmp", "tmpfs", 0, NULL);

	mknod_dev("/dev/kmsg", 1, 11, S_IFCHR | 0600);
	kfd = open("/dev/kmsg", O_WRONLY);

	klog("hmos-preinit (negroni) starting");
	load_modules();
	{
		int i, n = 0;
		for (i = 0; i < 25; i++) {
			n = block_count();
			if (n > 0)
				break;
			sleep(1);
		}
		klog("block devices after %ds: %d", i, n);
		if (n == 0)
			fail_stage(1);
	}
	uevent_all();
	cache_logger();
	if (read_file("/proc/cmdline", cmdline, sizeof(cmdline)) == 0)
		klog("cmdline: %s", cmdline);
	klog("androidboot.mode=%s", strstr(cmdline, "androidboot.mode=charger") ? "charger" : "normal");

	/* point /system at /usr/system before /usr is mounted; the symlink is
	 * dangling until then, which is fine.
	 */
	{
		struct stat st;
		if (lstat("/system", &st) == 0 && !S_ISLNK(st.st_mode)) {
			rename("/system", "/system.ramdisk");
			if (symlink("/usr/system", "/system"))
				klog("early symlink failed: %s", strerror(errno));
		}
	}

	rlog("init version 15, block devices=%d", block_count());

	/* device-mapper control node: our fresh tmpfs /dev has no /dev/mapper */
	{
		int fd;
		mkdir_p("/dev/mapper");
		if (mknod("/dev/mapper/control", S_IFCHR | 0600, makedev(10, 236)) &&
		    errno != EEXIST)
			rlog("mknod /dev/mapper/control failed: %s", strerror(errno));
		mknod("/dev/device-mapper", S_IFCHR | 0600, makedev(10, 236));
		fd = open("/dev/mapper/control", O_RDWR);
		rlog("/dev/mapper/control open=%d (%s)", fd,
		     fd < 0 ? strerror(errno) : "ok");
		if (fd >= 0)
			close(fd);
		fd = open("/dev/device-mapper", O_RDWR);
		rlog("/dev/device-mapper open=%d (%s)", fd,
		     fd < 0 ? strerror(errno) : "ok");
		if (fd >= 0)
			close(fd);
	}
	if (map_super()) {
		klog("super mapping FAILED");
		rlog("FAIL map_super");
		fail_stage(2);
	}
	/* the dm devices were created after the first uevent scan, so their
	 * /dev/block/dm-N nodes do not exist yet - create them now.
	 */
	uevent_block();
	{
		int n = 0, k;
		for (k = 0; k < 32; k++) {
			char p[128];
			snprintf(p, sizeof(p), "/dev/block/dm-%d", k);
			if (access(p, F_OK) == 0)
				n++;
		}
		rlog("dm nodes created: %d", n);
	}

	rlog("map ok, system_minor=%u", system_minor);
	if (!have_system) {
		klog("no system partition mapped");
		rlog("FAIL no system minor");
		fail_stage(3);
	}

	/* OpenHarmony layout: the system partition is mounted at /usr and
	 * /usr/system is bind-mounted at /system, which is what the partition's
	 * own symlinks (bin -> /system/bin) expect.
	 */
	if (have_system) {
		char sdev[64];
		snprintf(sdev, sizeof(sdev), "/dev/block/dm-%u", system_minor);
		mkdir_p("/usr");
		rlog("mounting %s at /usr", sdev);
		if (mount(sdev, "/usr", "erofs", MS_RDONLY, NULL)) {
			int e4 = errno;
			klog("mount /usr from %s failed: %s", sdev, strerror(e4));
			rlog("FAIL mount /usr: %s (%d)", strerror(e4), e4);
			fail_stage(4);
		}
		else
			klog("/usr mounted from %s", sdev);
		/* OpenHarmony's init expects these to exist and be writable */
	mkdir_p("/dev/pts");
	mount("devpts", "/dev/pts", "devpts", 0, "mode=0620");
	mkdir_p("/dev/shm");
	mount("tmpfs", "/dev/shm", "tmpfs", 0, "mode=1777");
	mkdir_p("/data");
	if (mount("tmpfs", "/data", "tmpfs", 0, "mode=0771") == 0)
		rlog("/data mounted as tmpfs");
	else
		rlog("/data mount failed: %s", strerror(errno));
	mkdir_p("/tmp");
	rlog("mount ok, exec OH init as PID 1");

	/* feed the hardware watchdog from a child: OpenHarmony's init does not
	 * pet the Qualcomm WDT, which otherwise resets the phone after ~80 s.
	 */
	{
		pid_t wdpid = fork();

		if (wdpid == 0) {
			int fd, i;
			char tick = 0;

			setsid();
			fd = open("/dev/watchdog", O_WRONLY);
			if (fd < 0)
				fd = open("/dev/watchdog0", O_WRONLY);
			rlog("wdt fd=%d", fd);
			for (i = 0; fd >= 0 && i < 120; i++) {
				(void)!write(fd, &tick, 1);
				if (i % 6 == 0)
					rlog("wdt ping %d", i);
				sleep(5);
			}
			rlog("wdt pinger done");
			_exit(0);
		}
		rlog("watchdog pinger pid=%d", (int)wdpid);
	}

	/* the ~80 s reset is the Qualcomm watchdog: its kthread comes from these
	 * modules.  Load them explicitly and record what happens.
	 */
	{
		const char *wdt[] = {
			"qcom_wdt_core.ko", "qcom_enhance_watchdog.ko",
			"gh_virt_wdt.ko", "qcom_soc_wdt.ko", "init_watchdog.ko",
			"qcom_cpu_vendor_hooks.ko", "phy-generic.ko",
		};
		size_t k;

		for (k = 0; k < sizeof(wdt) / sizeof(wdt[0]); k++) {
			char p[320];
			int fd, e = 0;

			snprintf(p, sizeof(p), "/lib/modules/%s", wdt[k]);
			fd = open(p, O_RDONLY | O_CLOEXEC);
			if (fd < 0) {
				rlog("wdtmod %s: missing", wdt[k]);
				continue;
			}
			if (syscall(SYS_finit_module, fd, "", 0) == 0)
				rlog("wdtmod %s: ok", wdt[k]);
			else if (errno == EEXIST)
				rlog("wdtmod %s: already", wdt[k]);
			else {
				e = errno;
				rlog("wdtmod %s: %s (%d)", wdt[k], strerror(e), e);
			}
			close(fd);
		}
		rlog("wdt module pass done");
	}

	/* relay the kernel log into logdump so the panic produced by OH init is
	 * not lost on reset
	 */
	{
		pid_t kpid = fork();

		if (kpid == 0) {
			int kfd, lfd, off = 0;
			char buf[2048];

			setsid();
			lfd = rlog_fd;
			kfd = open("/dev/kmsg", O_RDONLY | O_NONBLOCK);
			rlog("kmsg relay fd=%d", kfd);
			sleep(3);
			while (kfd >= 0) {
				ssize_t n = read(kfd, buf, sizeof(buf) - 1);

				if (n <= 0) {
					usleep(200000);
					continue;
				}
				buf[n] = 0;
				/* skip our own lines to avoid feedback */
				if (strstr(buf, "hmos-preinit") || strstr(buf, "wdt ping"))
					continue;
				if (lfd >= 0 && off + 256 < 55000) {
					char slot[256];
					size_t len = (size_t)n;

					if (len > 250)
						len = 250;
					memset(slot, 0, sizeof(slot));
					memcpy(slot, buf, len);
					(void)!pwrite(lfd, slot, sizeof(slot), 32768 + off);
					off += 256;
					fsync(lfd);
				}
			}
			_exit(0);
		}
		rlog("kmsg relay pid=%d", (int)kpid);
	}

	rlog("CONTROL: parking without exec");
	{
		int i;
		for (i = 0; i < 60; i++) {
			sleep(5);
			rlog("parked %ds", (i + 1) * 5);
		}
	}
	rlog("parked for 300s - no reset happened");
	sync();
	reboot(RB_POWER_OFF);
	rlog("PID1 exec failed: %s", strerror(errno));

	rlog("falling back to fork+exec");

	{
		pid_t pid = fork();
		int i;

		if (pid == 0) {
			execl("/system/bin/init", "/system/bin/init", NULL);
			execl("/usr/system/bin/init", "/system/bin/init", NULL);
			rlog("child exec failed: %s", strerror(errno));
			_exit(127);
		}
		rlog("child pid=%d", (int)pid);
		for (i = 0; i < 14; i++) {
			int status = 0;
			pid_t r = waitpid(pid, &status, WNOHANG);
			sleep(5);
			if (r == pid)
				rlog("child exited (status 0x%x) at %ds", status, (i + 1) * 5);
			else
				rlog("child alive at %ds", (i + 1) * 5);
			if (r == pid)
				break;
		}
		rlog("diagnostic done, powering off");
		sync();
		sleep(2);
		reboot(RB_POWER_OFF);
	}
	} else {
		klog("no system partition - cannot hand over to OpenHarmony");
	}

	cache_flush();
	klog("exec %s", REAL_INIT);
	sync();
	execl(REAL_INIT, REAL_INIT, NULL);
	{
		int e = errno;
		klog("exec %s failed: %s", REAL_INIT, strerror(e));
		rlog("FAIL exec %s: %s (%d)", REAL_INIT, strerror(e), e);
		rlog("  /system/bin/init X_OK=%d  /usr/system/bin/init X_OK=%d",
		     access("/system/bin/init", X_OK) == 0,
		     access("/usr/system/bin/init", X_OK) == 0);
		rlog("  /system/lib/ld-musl X_OK=%d",
		     access("/system/lib/ld-musl-aarch64.so.1", X_OK) == 0);
		sync();
		sleep(20);
		reboot(RB_POWER_OFF);
	}
	return 0;
}
