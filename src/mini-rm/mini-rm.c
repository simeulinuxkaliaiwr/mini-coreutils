#if !defined(__x86_64__)
#error "This project only supports Linux x86_64"
#endif

#include "lib.h"

int flag_recursive = 0;
int flag_force     = 0;
int flag_dir       = 0;

struct linux_dirent64 {
	ino64_t            d_ino;
	off64_t            d_off;
	unsigned short     d_reclen;
	unsigned char      d_type;
	char               d_name[];
};

#define DT_DIR 4

void remove_recursive(const char *path) {
	int fd = guicall(SYS_open, path, O_RDONLY | O_DIRECTORY);
	if (fd < 0) {
		if (!flag_force) {
			const char *err = "rm: cannot open directory: ";
			write(STDERR_FILENO, err, len(err));
			write(STDERR_FILENO, path, len(path));
			write(STDERR_FILENO, "\n", 1);
		}
		return;
	}

	char buf[4096];
	long nread;

	while ((nread = guicall(SYS_getdents64, fd, buf, sizeof(buf))) > 0) {
		for (long bpos = 0; bpos < nread; ) {
			struct linux_dirent64 *d = (struct linux_dirent64 *)(buf + bpos);
            
			if (cmp(d->d_name, ".") == 0 || cmp(d->d_name, "..") == 0) {
				bpos += d->d_reclen;
				continue;
			}
			
			unsigned long path_l = len(path);
			unsigned long name_l = len(d->d_name);

			char *subpath = (char *)malloc(path_l + name_l + 2);
			if (subpath == NULL) {
				const char* err = "mini-rm: out of memory\n";
				write(STDERR_FILENO, err, len(err));
				return;
			}

			scpy(subpath, path);
            
			if (subpath[path_l - 1] != '/') {
				subpath[path_l] = '/';
				subpath[path_l + 1] = '\0';
			}
            
			scat(subpath, d->d_name); 

			if (d->d_type == DT_DIR) {
				remove_recursive(subpath);
			} else {
				if (guicall(SYS_unlink, subpath) < 0 && !flag_force) {
					const char* msg = "rm: failed to remove file\n";
					write(STDERR_FILENO, msg, len(msg));
				}
			}
			free(subpath);
			bpos += d->d_reclen;
		}
	}

	guicall(SYS_close, fd);
	guicall(SYS_rmdir, path);
}

void process_target(const char *target) {
    // Tenta apagar como arquivo primeiro
	if (guicall(SYS_unlink, target) == 0) return;

    // Se falhou, verificamos se é diretório
	if (flag_recursive) {
		remove_recursive(target);
	} else if (flag_dir) {
		if (guicall(SYS_rmdir, target) < 0 && !flag_force) {
			const char* msg = "rm: directory not empty or access denied\n";
			write(STDERR_FILENO, msg, len(msg));
		}
	} else if (!flag_force) {
		const char* msg = "rm: cannot remove directory (use -r)\n";
		write(STDERR_FILENO, msg, len(msg));
	}
}

void c_start(long *sp) {
	int argc = (int)*sp;
	char **argv = (char **)(sp + 1);

	if (argc < 2) {
		const char *usage = "usage: rm [-rfd] targets...\n";
		write(STDERR_FILENO, usage, len(usage));
		guicall(SYS_exit, 1);
	}

	int i = 1;
	for (; i < argc; i++) {
		if (argv[i][0] == '-') {
			for (int j = 1; argv[i][j] != '\0'; j++) {
				if (argv[i][j] == 'r' || argv[i][j] == 'R') flag_recursive = 1;
				else if (argv[i][j] == 'f') flag_force = 1;
				else if (argv[i][j] == 'd') flag_dir = 1;
			}
		} else {
			break; 
		}
	}

	for (; i < argc; i++) {
		process_target(argv[i]);
	}

	guicall(SYS_exit, 0);
}

void __attribute__((naked)) _start() {
	asm volatile ("mov %rsp, %rdi\nandq $-16, %rsp\ncall c_start");
}
