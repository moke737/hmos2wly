// init_v12 = staged self-test whose *power-off delay* encodes the failure stage.
//
//   delay = 10 + 5*stage + module_failures   seconds
//
//   stage 1 = configfs mount failed
//   stage 2 = usb_gadget dir could not be created
//   stage 3 = acm function / symlink failed
//   stage 4 = no UDC (dwc3 did not probe)
//   stage 5 = UDC bound but /dev/ttyGS0 missing
//   stage 0 = everything OK  -> never powers off (stays alive, COM port up)
//
// So "phone powered off after ~N s" tells us the stage and the module failure
// count without any log channel at all.

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

static int kfd = -1, rawfd = -1, tfd = -1;
static off_t roff;
static int n_ok, n_fail;
static int stage;

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
	if (tfd >= 0 && write(tfd, line, n) < 0)
		;
	if (rawfd >= 0 && pwrite(rawfd, line, n, roff) == (ssize_t)n)
		roff += n;
}

static void mkdir_p(const char *p)
{
	char t[220];
	size_t i;

	snprintf(t, sizeof(t), "%s", p);
	for (i = 1; t[i]; i++)
		if (t[i] == '/') { t[i] = 0; mkdir(t, 0755); t[i] = '/'; }
	mkdir(t, 0755);
}

static int wr(const char *path, const char *val)
{
	int fd = open(path, O_WRONLY);

	if (fd < 0)
		return -1;
	if (write(fd, val, strlen(val)) < 0) {
		close(fd);
		return -1;
	}
	close(fd);
	return 0;
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

static void load_modules(void)
{
	FILE *f = fopen("/lib/modules/modules.load", "r");
	char line[256];

	if (!f)
		return;
	while (fgets(line, sizeof(line), f)) {
		char path[320];
		int fd;

		line[strcspn(line, "\r\n")] = 0;
		if (!line[0] || line[0] == '#')
			continue;
		snprintf(path, sizeof(path), "/lib/modules/%s", line);
		fd = open(path, O_RDONLY | O_CLOEXEC);
		if (fd < 0) { n_fail++; continue; }
		if (syscall(SYS_finit_module, fd, "", 0) == 0)
			n_ok++;
		else if (errno != EEXIST)
			n_fail++;
		close(fd);
	}
	fclose(f);
}

static void gadget(void)
{
	DIR *d;
	struct dirent *e;
	char udc[128] = "";
	char p[256];

	mkdir_p("/sys/kernel/config");
	if (mount("configfs", "/sys/kernel/config", "configfs", 0, NULL) != 0 && errno != EBUSY) {
		stage = 1;
		return;
	}

	mkdir_p("/sys/kernel/config/usb_gadget/hmos");
	if (access("/sys/kernel/config/usb_gadget/hmos", F_OK) != 0) {
		stage = 2;
		return;
	}
	wr("/sys/kernel/config/usb_gadget/hmos/idVendor", "0x18d1");
	wr("/sys/kernel/config/usb_gadget/hmos/idProduct", "0xd00d");
	wr("/sys/kernel/config/usb_gadget/hmos/bcdUSB", "0x0200");
	mkdir_p("/sys/kernel/config/usb_gadget/hmos/strings/0x409");
	wr("/sys/kernel/config/usb_gadget/hmos/strings/0x409/serialnumber", "hmos2negroni");
	wr("/sys/kernel/config/usb_gadget/hmos/strings/0x409/manufacturer", "hmos");
	wr("/sys/kernel/config/usb_gadget/hmos/strings/0x409/product", "hmos-diag");
	mkdir_p("/sys/kernel/config/usb_gadget/hmos/configs/c.1/strings/0x409");
	wr("/sys/kernel/config/usb_gadget/hmos/configs/c.1/strings/0x409/configuration", "diag");

	if (access("/sys/kernel/config/usb_gadget/hmos/functions", F_OK) != 0) {
		stage = 3;
		return;
	}
	mkdir_p("/sys/kernel/config/usb_gadget/hmos/functions/acm.GS0");
	if (access("/sys/kernel/config/usb_gadget/hmos/functions/acm.GS0", F_OK) != 0) {
		stage = 3;
		return;
	}
	snprintf(p, sizeof(p), "/sys/kernel/config/usb_gadget/hmos/configs/c.1/acm.GS0");
	if (symlink("/sys/kernel/config/usb_gadget/hmos/functions/acm.GS0", p) != 0 && errno != EEXIST) {
		stage = 3;
		return;
	}

	d = opendir("/sys/class/udc");
	if (d) {
		while ((e = readdir(d))) {
			if (e->d_name[0] == '.')
				continue;
			snprintf(udc, sizeof(udc), "%s", e->d_name);
			break;
		}
		closedir(d);
	}
	if (!udc[0]) {
		stage = 4;
		return;
	}
	if (wr("/sys/kernel/config/usb_gadget/hmos/UDC", udc) != 0) {
		stage = 4;
		return;
	}
	sleep(2);
	tfd = open("/dev/ttyGS0", O_RDWR | O_NONBLOCK);
	if (tfd < 0) {
		stage = 5;
		return;
	}
	stage = 0;
}

int main(void)
{
	char dev[96];
	int delay;

	mkdir_p("/proc"); mkdir_p("/sys"); mkdir_p("/dev"); mkdir_p("/tmp");
	mount("proc", "/proc", "proc", 0, NULL);
	mount("sysfs", "/sys", "sysfs", 0, NULL);
	mount("tmpfs", "/dev", "tmpfs", 0, "mode=0755");
	mount("tmpfs", "/tmp", "tmpfs", 0, NULL);
	node("/dev/kmsg", 1, 11, S_IFCHR | 0600);
	node("/dev/null", 1, 3, S_IFCHR | 0666);
	kfd = open("/dev/kmsg", O_WRONLY);

	if (part_node("logdump", dev, sizeof(dev)) == 0)
		rawfd = open(dev, O_WRONLY);

	out("init_v12: begin");
	load_modules();
	out("modules ok=%d fail=%d", n_ok, n_fail);
	gadget();
	out("gadget stage=%d", stage);

	if (stage == 0) {
		out("USB OK - staying alive");
		for (;;)
			pause();
	}

	delay = 10 + 5 * stage + n_fail;
	if (delay > 120)
		delay = 120;
	out("stage=%d fail=%d -> power off in %d s", stage, n_fail, delay);
	sleep(delay);
	reboot(RB_POWER_OFF);
	for (;;)
		pause();
}
