// init_v8 = init_v7 + pstore dump.
//
// If the previous boot of this kernel panicked, the OEM dtb's ramoops_region
// still holds the dmesg; we copy it into the raw logdump partition before doing
// anything else, then continue with the storage report.

#define _GNU_SOURCE
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <unistd.h>

static int kfd = -1, rawfd = -1;
static off_t roff;
static int n_ok, n_fail;

static void out(const char *fmt, ...)
{
	char b[1024], line[1100];
	va_list ap;
	int n;

	va_start(ap, fmt);
	n = vsnprintf(b, sizeof(b), fmt, ap);
	va_end(ap);
	if (n < 0)
		return;
	n = snprintf(line, sizeof(line), "%s\n", b);
	if (kfd >= 0 && write(kfd, line, n) < 0)
		;
	if (rawfd >= 0 && pwrite(rawfd, line, n, roff) == (ssize_t)n)
		roff += n;
}

static void mkdir_p(const char *p)
{
	char t[160];
	size_t i;

	snprintf(t, sizeof(t), "%s", p);
	for (i = 1; t[i]; i++)
		if (t[i] == '/') { t[i] = 0; mkdir(t, 0755); t[i] = '/'; }
	mkdir(t, 0755);
}

static int node(const char *p, int maj, int min, mode_t mode)
{
	if (mknod(p, mode, makedev(maj, min)) == 0)
		return 0;
	return errno == EEXIST ? 0 : -errno;
}

static int part_node(const char *partname, char *o, size_t olen)
{
	DIR *d = opendir("/sys/class/block");
	struct dirent *e;

	if (!d)
		return -1;
	while ((e = readdir(d))) {
		char p[320], line[512], found[64] = "";
		FILE *f;
		int maj = -1, min = -1;

		if (e->d_name[0] == '.')
			continue;
		snprintf(p, sizeof(p), "/sys/class/block/%s/uevent", e->d_name);
		f = fopen(p, "r");
		if (!f)
			continue;
		while (fgets(line, sizeof(line), f)) {
			line[strcspn(line, "\r\n")] = 0;
			if (!strncmp(line, "MAJOR=", 6))
				maj = atoi(line + 6);
			else if (!strncmp(line, "MINOR=", 6))
				min = atoi(line + 6);
			else if (!strncmp(line, "PARTNAME=", 9))
				snprintf(found, sizeof(found), "%s", line + 9);
		}
		fclose(f);
		if (strcmp(found, partname) || maj < 0 || min < 0)
			continue;
		snprintf(o, olen, "/dev/%s-blk", partname);
		node(o, maj, min, S_IFBLK | 0600);
		closedir(d);
		return 0;
	}
	closedir(d);
	return -1;
}

static void dump_pstore(void)
{
	DIR *d;
	struct dirent *e;

	mkdir_p("/sys/fs/pstore");
	if (mount("pstore", "/sys/fs/pstore", "pstore", 0, NULL) != 0 && errno != EBUSY)
		out("pstore: mount failed (errno %d)", errno);
	d = opendir("/sys/fs/pstore");
	if (!d) {
		out("pstore: unavailable");
		return;
	}
	out("=== pstore (previous boot) ===");
	while ((e = readdir(d))) {
		char p[256];
		int fd, n;
		char buf[2048];

		if (e->d_name[0] == '.')
			continue;
		out("--- %s ---", e->d_name);
		snprintf(p, sizeof(p), "/sys/fs/pstore/%s", e->d_name);
		fd = open(p, O_RDONLY);
		if (fd < 0)
			continue;
		while ((n = read(fd, buf, sizeof(buf))) > 0) {
			if (rawfd >= 0) {
				if (pwrite(rawfd, buf, n, roff) == n)
					roff += n;
			}
		}
		close(fd);
		out("--- end %s ---", e->d_name);
	}
	closedir(d);
	out("=== end pstore ===");
}

static void load_modules(void)
{
	FILE *f = fopen("/lib/modules/modules.load", "r");
	char line[256];

	if (!f) {
		out("modules.load: MISSING");
		return;
	}
	while (fgets(line, sizeof(line), f)) {
		char path[320];
		int fd;

		line[strcspn(line, "\r\n")] = 0;
		if (!line[0] || line[0] == '#')
			continue;
		snprintf(path, sizeof(path), "/lib/modules/%s", line);
		fd = open(path, O_RDONLY | O_CLOEXEC);
		if (fd < 0) {
			n_fail++;
			continue;
		}
		if (syscall(SYS_finit_module, fd, "", 0) == 0)
			n_ok++;
		else if (errno != EEXIST)
			n_fail++;
		close(fd);
	}
	fclose(f);
}

int main(void)
{
	char dev[96];
	int nblk = 0;
	DIR *d;
	struct dirent *e;

	mkdir_p("/proc"); mkdir_p("/sys"); mkdir_p("/dev"); mkdir_p("/tmp");
	mount("proc", "/proc", "proc", 0, NULL);
	mount("sysfs", "/sys", "sysfs", 0, NULL);
	mount("tmpfs", "/dev", "tmpfs", 0, "mode=0755");
	mount("tmpfs", "/tmp", "tmpfs", 0, NULL);
	node("/dev/kmsg", 1, 11, S_IFCHR | 0600);
	node("/dev/null", 1, 3, S_IFCHR | 0666);
	kfd = open("/dev/kmsg", O_WRONLY);

	/* open logdump as early as possible */
	if (part_node("logdump", dev, sizeof(dev)) == 0)
		rawfd = open(dev, O_WRONLY);

	out("HMOS2NEGRONI-V8");
	out("kernel: %s", "lineageos 5.10 build");
	dump_pstore();

	out("modules.load: loading ...");
	load_modules();
	out("modules: ok=%d fail=%d", n_ok, n_fail);

	d = opendir("/sys/class/block");
	if (d) {
		while ((e = readdir(d)))
			if (e->d_name[0] != '.')
				nblk++;
		closedir(d);
	}
	out("block devices: %d", nblk);
	{
		char sd[96];
		out("super: %s", part_node("super", sd, sizeof(sd)) == 0 ? "PRESENT" : "missing");
	}
	out("=== partitions ===");
	d = opendir("/sys/class/block");
	if (d) {
		while ((e = readdir(d))) {
			char p[320], line[256], nm[64] = "";
			FILE *f;

			if (e->d_name[0] == '.')
				continue;
			snprintf(p, sizeof(p), "/sys/class/block/%s/uevent", e->d_name);
			f = fopen(p, "r");
			if (!f)
				continue;
			while (fgets(line, sizeof(line), f)) {
				line[strcspn(line, "\r\n")] = 0;
				if (!strncmp(line, "PARTNAME=", 9))
					snprintf(nm, sizeof(nm), "%s", line + 9);
			}
			fclose(f);
			if (nm[0])
				out("  %s", nm);
		}
		closedir(d);
	}
	out("=== end ===");
	sync();
	out("init_v8: powering off");
	sleep(1);
	reboot(RB_POWER_OFF);
	for (;;)
		pause();
}
