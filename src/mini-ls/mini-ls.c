#if !defined(__x86_64__)
#error "This project only supports Linux x86_64"
#endif

#include "lib.h"

struct linux_dirent64 {
	ino64_t d_ino;
	off64_t d_off;
	unsigned short d_reclen;
	unsigned char d_type;
	char d_name[];
};

struct stat64 {
	uint64_t st_dev;
	uint64_t st_ino;
	uint64_t st_nlink;
	uint32_t st_mode;
	uint32_t st_uid;
	uint32_t st_gid;
	uint32_t __pad0;
	uint64_t st_rdev;
	int64_t  st_size;
	int64_t  st_blksize;
	int64_t  st_blocks;
	uint64_t st_atime;
	uint64_t st_atime_nsec;
	uint64_t st_mtime;
	uint64_t st_mtime_nsec;
	uint64_t st_ctime;
	uint64_t st_ctime_nsec;
	int64_t __unused[3];
};

void show_help()
{
	const char* msg =
		"mini-ls made by simeulinuxkaliaiwr\n"
		"Uso: mini-ls [OPTIONS] [PATH]\n\n"
		"Options:\n"
		"-l, --long	use a long listing format (mode, uid, gid, size, etc)\n"
		"-a, --all	do not ignore entries starting with . or ..\n"
		"-r, --recursive list subdirectories recursively\n\n"
		"Example:\n"
		"./mini-ls -la /etc\n"
		"./mini-ls -r /home/user\n";
	write(STDOUT_FILENO, msg, len(msg));
	exit_asm(0)
}

typedef struct {
	int all;
	int long_format;
	int recursive;
	const char* path;
} ls_options_t;

void parse_args(int argc, char **argv, ls_options_t *opts)
{
	int i;
	for (i = 1;i < argc;i++) {
		const char* arg = argv[i];

		if (arg[0] == '-' && arg[1] == '-') {
			if (cmp(arg, "--all") == 0) {
				opts->all = 1;
			} else if (cmp(arg, "--recursive") == 0) {
				opts->recursive = 1;
			}
			continue;
		}

		/* Short flags */
		if (arg[0] == '-' && arg[1] != '\0') {
			const char* p = &arg[1];
			while (*p) {
				switch(*p) {
					case 'a': opts->all = 1; break;
					case 'l': opts->long_format = 1; break;
					case 'R': opts->recursive = 1; break;
					default: show_help(); break;
				}
				p++;
			}
			continue;
		}

		if (!opts->path) {
			opts->path = arg;
		}
	}

	if (!opts->path) {
		opts->path = ".";
	}
}

void write_num(int64_t num)
{
	char buf[32];
	int i = 0;
	int neg = 0;

	if (num < 0) {
		neg = 1;
		num = -num;
	}

	if (num == 0) {
		buf[i++] = '0';
	} else {
		while (num > 0) {
			buf[i++] = '0' + (num % 10);
			num /= 10;
		}
	}

	if (neg) {
		buf[i++] = '-';
	}

	for (int j = i - 1;j >= 0;--j) {
		write(STDOUT_FILENO, &buf[j], 1);
	}
}

void write_mode(uint32_t mode)
{
	char perms[11] = "----------";

	if ((mode & 0170000) == 0040000)
		perms[0] = 'd';
	if ((mode & 0170000) == 0120000)
		perms[0] = 'l';
	
	if (mode & 0400)
		perms[1] = 'r';
	if (mode & 0200)
		perms[2] = 'w';
	if (mode & 0100)
		perms[3] = 'x';
	if (mode & 0040)
		perms[4] = 'r';
	if (mode & 0020)
		perms[5] = 'w';
	if (mode & 0010)
		perms[6] = 'x';
	if (mode & 0004)
		perms[7] = 'r';
	if (mode & 0002)
		perms[8] = 'w';
	if (mode & 0001)
		perms[9] = 'x';

	write(STDOUT_FILENO, perms, 10);
}

void show_long(const char* path, const char* name)
{
	char full[4096];
	scpy(full, path);
	if (full[len(full) - 1] != '/') {
		scat(full, "/");
	}
	scat(full, name);

	struct stat64 info;
	if (guicall(SYS_stat, full, &info) < 0) {
		return;
	}

	write_mode(info.st_mode);
	write(STDOUT_FILENO, " ", 1);
	write_num(info.st_nlink);
	write(STDOUT_FILENO, " ", 1);
	write_num(info.st_uid);
	write(STDOUT_FILENO, " ", 1);
	write_num(info.st_gid);
	write(STDOUT_FILENO, " ", 1);
	write_num(info.st_size);
	write(STDOUT_FILENO, " ", 1);
	write(STDOUT_FILENO, name, len(name));

	if ((info.st_mode & 0170000) == 0120000) {
		char target[4096];
		int len = guicall(SYS_readlink, full, target, sizeof(target) - 1);
		if (len > 0) {
			target[len] = '\0';
			write(STDOUT_FILENO, " ->", 4);
			write(STDOUT_FILENO, target, len);
		}
	}

	write(STDOUT_FILENO, "\n", 1);
}

void list(const char* path, ls_options_t *opt); /* Prototype */

void scan_recursive(const char* path, ls_options_t *opt)
{
	int fd = guicall(SYS_open, path, O_RDONLY | O_DIRECTORY);
	if (fd < 0) return;

	unsigned long buf_size = (1024 * 32);
	char *buf = (char *)guicall(SYS_mmap, NULL, buf_size, PROT_READ | PROT_WRITE,
				MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if (buf == MAP_FAILED) {
		guicall(SYS_close, fd);
		return;
	}

	long nread;
	while ((nread = guicall(SYS_getdents64, fd, buf, buf_size)) > 0) {
		for (unsigned long bpos = 0;bpos < (unsigned long)nread;) {
			struct linux_dirent64 *d = (void *)(buf + bpos);

			if (cmp(d->d_name, ".") != 0 && cmp(d->d_name, "..") != 0) {
				if (d->d_type == 4) {
					char subpath[4096];
					scpy(subpath, path);
					if (subpath[len(subpath) - 1] != '/') scat(subpath, "/");
					scat(subpath, d->d_name);

					write(STDOUT_FILENO, "\n", 1);
					write(STDOUT_FILENO, subpath, len(subpath));
					write(STDOUT_FILENO, ":\n", 2);

					list(subpath, opt);
				}
			}

			bpos += d->d_reclen;
		}
	}

	guicall(SYS_close, fd);
	guicall(SYS_munmap, buf, buf_size);
}

void list(const char* path, ls_options_t *opt)
{
	int fd = guicall(SYS_open, path, O_RDONLY | O_DIRECTORY);
	if (fd < 0) {
		int err = extract_error(fd);
		const char* msg = strerror(err);
		
		write(STDERR_FILENO, "ls: cannot access '", 19);
		write(STDERR_FILENO, path, len(path));
		write(STDERR_FILENO, "': ", 3);
		write(STDERR_FILENO, msg, len(msg));
		write(STDERR_FILENO, "\n", 1);
		exit_asm(1);
	}

	unsigned long buf_size = (1024 * 32);
	char *buf = (char *)guicall(SYS_mmap, NULL, buf_size, PROT_READ | PROT_WRITE,
			MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
	if (buf == MAP_FAILED) {
		const char* msg = "mmap syscall failed.\n";
		write(STDERR_FILENO, msg, len(msg));
		guicall(SYS_close, fd);
		exit_asm(1);
	}

	long nread;
	while ((nread = guicall(SYS_getdents64, fd, buf, buf_size)) > 0) {
		for (unsigned long bpos = 0;bpos < (unsigned long)nread;) {
			struct linux_dirent64 *d = (void *)(buf + bpos);
			int hidden = d->d_name[0] == '.';

			if (!opt->all && hidden) {
				bpos += d->d_reclen;
				continue;
			}

			if (cmp(d->d_name, ".") != 0 && cmp(d->d_name, "..") != 0) {
				if (opt->long_format) {
					show_long(path, d->d_name);
				} else {
					write(STDOUT_FILENO, d->d_name, len(d->d_name));
					if (d->d_type == 4) {
						write(STDOUT_FILENO, "/", 1);
					}
					write(STDOUT_FILENO, "\n", 1);
				}
			}
			bpos += d->d_reclen;
		}
	}

	guicall(SYS_close, fd);

	if (opt->recursive) {
		scan_recursive(path, opt);
	}

	guicall(SYS_munmap, buf, buf_size);
}

void c_start(long *sp)
{
	int argc = (int)*sp;
	char **argv = (char **)(sp + 1);

	ls_options_t opts = {0};

	parse_args(argc, argv, &opts);

	list(opts.path, &opts);

	exit_asm(0);
}

void __attribute__((naked)) _start()
{
	asm volatile (
		"mov %rsp, %rdi\n"
		"andq $-16, %rsp\n"
		"call c_start"
	);
}

