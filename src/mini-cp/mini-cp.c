#include "lib.h"

#if !defined(__x86_64__)
#error "This project only supports Linux x86_64"
#endif

#define BUFFER_SIZE (64 * 1024)
#define PATH_MAX 4096

struct linux_dirent64 {
	ino64_t d_ino;
	off64_t d_off;
	unsigned short d_reclen;
	unsigned char d_type;
	char d_name[];
};

typedef struct {
	int recursive;
	int force;
	int interactive;
	int verbose;
	int preserve;
	int no_dereference;
	int update;
	int backup;
	char backup_suffix[8];
} cp_options_t;

typedef struct {
	uint64_t files_copied;
	uint64_t directories_created;
	uint64_t bytes_copied;
	uint64_t errors;
} cp_stats_t;

int copy_file(const char* src, const char* dst, const cp_options_t *opts, cp_stats_t *stats);
int copy_directory(const char* src, const char* dst, const cp_options_t *opts, cp_stats_t *stats);
int prompt_user(const char* msg);
void preserve_metadata(const char* src, const char* dst);
void show_progress(uint64_t current, uint64_t total, const char* filename);
char* make_backup_name(const char* filename, const char* suffix);

int is_directory(const char* path)
{
	struct stat64 st;
	if (guicall(SYS_stat, path, &st) < 0) return 0;
	return (st.st_mode & S_IFMT) == S_IFDIR;
}

int path_exists(const char* path)
{
	return guicall(SYS_access, path, F_OK) == 0;
}

int create_parent_dirs(const char* path)
{
	char temp[PATH_MAX];
	char *p;

	scpy(temp, path);

	for (p = temp + len(temp);p > temp;p--) {
		if (*p == '/') {
			*p = '\0';
			break;
		}
	}

	if (len(temp) == 0 || cmp(temp, ".") == 0 || cmp(temp, "..") == 0) {
		return 0;
	}
	
	long ret = guicall(SYS_mkdir, temp, 0755);
	if (ret < 0) {
		int err = extract_error(ret);
		if (err == EEXIST) return 0;
		if (err == ENOENT) {
			create_parent_dirs(temp);
			return guicall(SYS_mkdir, temp, 0755) == 0 ? 0 : -1;
		}
		return -1;
	}
	return 0;
}

int copy_regular_file(const char* src, const char* dst, const cp_options_t *opts, cp_stats_t *stats)
{
	int src_fd, dst_fd;
	char *buffer;
	long bytes_read, bytes_written;
	uint64_t total_copied = 0;

	src_fd = guicall(SYS_open, src, O_RDONLY);
	if (src_fd < 0) {
		if (opts->verbose) {
			const char* msg = "mini-cp: cannot open '";
			write(STDERR_FILENO, msg, len(msg));
			write(STDERR_FILENO, src, len(src));
			write(STDERR_FILENO, "' for reading\n", 14);
		}
		return -1;
	}

	struct stat64 st;
	if (guicall(SYS_fstat, src_fd, &st) < 0) {
		guicall(SYS_close, src_fd);
		return -1;
	}
	uint64_t file_size = st.st_size;

	if (path_exists(dst)) {
		if (opts->interactive) {
			char prompt[512];
			scpy(prompt, "mini-cp: overwrite '");
			scat(prompt, dst);
			scat(prompt, "'? ");

			if (!prompt_user(prompt)) {
				guicall(SYS_close, src_fd);
				return 0;
			}
		}

		if (opts->update) {
			struct stat64 dst_st;
			if (guicall(SYS_stat, dst, &dst_st) == 0) {
				if (st.st_mtime.tv_sec < dst_st.st_mtime.tv_sec || (st.st_mtime.tv_sec == dst_st.st_mtime.tv_sec && st.st_mtime.tv_nsec <= dst_st.st_mtime.tv_nsec)) {
					if (opts->verbose) {
						const char* msg = "mini-cp: skipping '";
						write(STDOUT_FILENO, msg, len(msg));
						write(STDOUT_FILENO, src, len(src));
						write(STDOUT_FILENO, "' (not newer)\n", 14);
					}
					guicall(SYS_close, src_fd);
					return 0;
				}
			}
		}

		if (opts->backup) {
			char *backup = make_backup_name(dst, opts->backup_suffix);
			if (backup) {
				guicall(SYS_rename, dst, backup);
				if (opts->verbose) {
					const char* msg = "mini-cp: backed up '";
					write(STDOUT_FILENO, msg, len(msg));
					write(STDOUT_FILENO, dst, len(dst));
					write(STDOUT_FILENO, "' to '", 6);
					write(STDOUT_FILENO, backup, len(backup));
					write(STDOUT_FILENO, "'\n", 2);
				}
				free(backup);
			}
		}

		if (!opts->force) {
			if (guicall(SYS_access, dst, W_OK) < 0) {
				if (!opts->interactive) {
					const char* msg = "mini-cp: cannot overwrite '";
					write(STDERR_FILENO, msg, len(msg));
					write(STDERR_FILENO, dst, len(dst));
					write(STDERR_FILENO, "': Permission denied\n", 21);
					guicall(SYS_close, src_fd);
					return -1;
				}
			}
		}
	}

	create_parent_dirs(dst);

	dst_fd = guicall(SYS_open, dst, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (dst_fd < 0) {
		if (opts->verbose) {
			const char* msg = "mini-cp: cannot open '";
			write(STDERR_FILENO, msg, len(msg));
			write(STDERR_FILENO, dst, len(dst));
			write(STDERR_FILENO, "' for writing\n", 14);
		}
		guicall(SYS_close, src_fd);
		return -1;
	}

	buffer = malloc(BUFFER_SIZE);
	if (!buffer) {
		const char* msg = "mini-cp: out of memory\n";
		write(STDERR_FILENO, msg, len(msg));
		guicall(SYS_close, src_fd);
		guicall(SYS_close, dst_fd);
		return -1;
	}

	while ((bytes_read = guicall(SYS_read, src_fd, buffer, BUFFER_SIZE)) > 0) {
		bytes_written = write(dst_fd, buffer, bytes_read);
		if (bytes_written != bytes_read) {
			const char* msg = "mini-cp: write error\n";
			write(STDERR_FILENO, msg, len(msg));
			free(buffer);
			guicall(SYS_close, src_fd);
			guicall(SYS_close, dst_fd);
			return -1;
		}
		total_copied += bytes_written;

		if (opts->verbose && file_size > 0) {
			show_progress(total_copied, file_size, src);
		}
	}

	if (bytes_read < 0) {
		const char* msg = "mini-cp: read error\n";
		write(STDERR_FILENO, msg, len(msg));
		free(buffer);
		guicall(SYS_close, src_fd);
		guicall(SYS_close, dst_fd);
		return -1;
	}

	if (opts->verbose && file_size > 0) {
		write(STDOUT_FILENO, "\n", 1);
	}

	free(buffer);
	guicall(SYS_close, src_fd);
	guicall(SYS_close, dst_fd);

	if (opts->preserve) {
		preserve_metadata(src, dst);
	}

	stats->files_copied++;
	stats->bytes_copied += total_copied;

	if (opts->verbose) {
		char msg[128];
		scpy(msg, "mini-cp: copied '");
		scat(msg, src);
		scat(msg, "' -> '");
		scat(msg, dst);
		scat(msg, "' (");

		char num_str[32];
		char *p = num_str + 31;
		*p = '\0';
		uint64_t n = total_copied;
		do {
			*--p = '0' + (n % 10);
			n /= 10;
		} while (n > 0);
		scat(msg, p);
		scat(msg, " bytes)\n");
		write(STDOUT_FILENO, msg, len(msg));
	}

	return 0;
}

void preserve_metadata(const char* src, const char* dst)
{
	struct stat64 st;
	if (guicall(SYS_stat, src, &st) < 0) return;

	guicall(SYS_chmod, dst, st.st_mode & 07777);

	struct timespec times[2];
	times[0].tv_sec = st.st_atime.tv_sec;
	times[0].tv_nsec = st.st_atime.tv_nsec;

	times[1].tv_sec = st.st_mtime.tv_sec;
	times[1].tv_nsec = st.st_mtime.tv_nsec;

	guicall(SYS_utimensat, AT_FDCWD, dst, times, 0);
}
void show_progress(uint64_t current, uint64_t total, const char* filename)
{
	static int last_percent = -1;
	int percent = (int)((current * 100) / total);

	if (percent != last_percent) {
		char progress[256];
		scpy(progress, "\r");
		scat(progress, filename);
		scat(progress, ": ");

		scat(progress, "[");
		int bars = (percent * 20) / 100;
		for (int i = 0;i < 20;++i) {
			if (i < bars) scat(progress, "=");
			else if (i == bars) scat(progress, ">");
			else scat(progress, " ");
		}
		scat(progress, "] ");

		char percent_str[5];
		char *p = percent_str + 4;
		*p = '\0';
		int n = percent;
		do {
			*--p = '0' + (n % 10);
			n /= 10;
		} while (n > 0);
		scat(progress, p);
		scat(progress, "%");

		write(STDERR_FILENO, progress, len(progress));
		last_percent = percent;
	}

	if (percent == 100) {
		write(STDERR_FILENO, "\n", 1);
		last_percent = -1;
	}
}

int prompt_user(const char* msg)
{
	write(STDERR_FILENO, msg, len(msg));

	char response[4];
	long n = guicall(SYS_read, STDIN_FILENO, response, sizeof(response));

	if (n > 0) {
		if (response[0] == 'y' || response[0] == 'Y') {
			return 1;
		}
	}
	return 0;
}

char* make_backup_name(const char* filename, const char* suffix)
{
	int len_name = len(filename);
	int len_suffix = len(suffix);
	char* backup = malloc(len_name + len_suffix + 1);
	if (!backup) return NULL;

	scpy(backup, filename);
	scat(backup, suffix);
	return backup;
}

int process_item(const char* src, const char* dst, const cp_options_t *opts, cp_stats_t *stats)
{
	struct stat64 st;

	int stat_result = opts->no_dereference ? guicall(SYS_lstat, src, &st) : guicall(SYS_stat, src, &st);

	if (stat_result < 0) {
		if (opts->verbose) {
			const char* msg = "mini-cp: cannot stat '";
			write(STDERR_FILENO, msg, len(msg));
			write(STDERR_FILENO, src, len(src));
			write(STDERR_FILENO, "'\n", 2);
		}
		stats->errors++;
		return -1;
	}

	if ((st.st_mode & S_IFMT) == S_IFDIR) {
		if (opts->recursive) {
			return copy_directory(src, dst, opts, stats);
		} else {
			const char* msg = "mini-cp: -r not specified; omitting directory '";
			write(STDERR_FILENO, msg, len(msg));
			write(STDERR_FILENO, src, len(src));
			write(STDERR_FILENO, "'\n", 2);
			return -1;
		}
	} else if ((st.st_mode & S_IFMT) == S_IFLNK && !opts->no_dereference) {
		char target[PATH_MAX];
		long len = guicall(SYS_readlink, src, target, sizeof(target) - 1);
		if (len > 0) {
			target[len] = '\0';
			if (guicall(SYS_symlink, target, dst) < 0) {
				stats->errors++;
				return -1;
			}
			stats->files_copied++;
			return 0;
		}
	} else {
		return copy_regular_file(src, dst, opts, stats);
	}

	return -1;
}

int copy_directory(const char* src, const char* dst, const cp_options_t *opts, cp_stats_t *stats)
{
	long ret = guicall(SYS_mkdir, dst, 0755);
	if (ret < 0 && extract_error(ret) != EEXIST) {
		if (opts->verbose) {
			const char* msg = "mini-cp: cannot create directory '";
			write(STDERR_FILENO, msg, len(msg));
			write(STDERR_FILENO, dst, len(dst));
			write(STDERR_FILENO, "'\n", 2);
		}
		stats->errors++;
		return -1;
	} else {
		stats->directories_created++;
	}

	int dir_fd = guicall(SYS_open, src, O_RDONLY | O_DIRECTORY);
	if (dir_fd < 0) {
		stats->errors++;
		return -1;
	}

	char buffer[8192];
	long nread;

	while ((nread = guicall(SYS_getdents64, dir_fd, buffer, sizeof(buffer))) > 0) {
		for (long bpos = 0;bpos < nread; ) {
			struct linux_dirent64 *d = (struct linux_dirent64 *)(buffer + bpos);

			if (cmp(d->d_name, ".") == 0 || cmp(d->d_name, "..") == 0) {
				bpos += d->d_reclen;
				continue;
			}

			char src_path[PATH_MAX], dst_path[PATH_MAX];

			scpy(src_path, src);
			if (src_path[len(src_path)-1] != '/') scat(src_path, "/");
			scat(src_path, d->d_name);

			scpy(dst_path, dst);
			if (dst_path[len(dst_path)-1] != '/') scat(dst_path, "/");
			scat(dst_path, d->d_name);

			process_item(src_path, dst_path, opts, stats);

			bpos += d->d_reclen;
		}
	}

	guicall(SYS_close, dir_fd);
	return 0;
}

void show_help()
{
	const char* help_msg =
		"mini-cp - Copy files and directories\n"
		"Usage: mini-cp [OPTION]... SOURCE... DIRECTORY\n"
		"       mini-cp [OPTION]... SOURCE DEST\n"
		"       mini-cp [OPTION]... -t DIRECTORY SOURCE...\n\n"
		"Options:\n"
		"  -f, --force           remove existing destination files\n"
		"  -i, --interactive     prompt before overwrite\n"
		"  -P, --no-dereference  never follow symbolic links\n"
		"  -p, --preserve        preserve file attributes\n"
		"  -r, -R, --recursive   copy directories recursively\n"
		"  -u, --update          copy only when SOURCE is newer\n"
		"  -v, --verbose         explain what is being done\n"
		"      --backup[=SUFFIX] make backup before overwriting\n"
		"      --help            display this help and exit\n\n"
		"Examples:\n"
		"  mini-cp file1 file2        Copy file1 to file2\n"
		"  mini-cp -r dir1 dir2       Copy dir1 recursively to dir2\n"
		"  mini-cp -i *.txt backup/   Copy all .txt files to backup/\n";

	write(STDOUT_FILENO, help_msg, len(help_msg));
}

void parse_args(int argc, char **argv, cp_options_t *opts)
{
	opts->recursive = 0;
	opts->force = 0;
	opts->interactive = 0;
	opts->verbose = 0;
	opts->preserve = 0;
	opts->no_dereference = 0;
	opts->update = 0;
	opts->backup = 0;
	scpy(opts->backup_suffix, "~");

	int i;
	for (i = 1;i < argc;i++) {
		if (argv[i][0] == '-') {
			if (cmp(argv[i], "--help") == 0) {
				show_help();
				exit_asm(0);
			} else if (cmp(argv[i], "--force") == 0 || cmp(argv[i], "-f") == 0) {
				opts->force = 1;
			} else if (cmp(argv[i], "--interactive") == 0 || cmp(argv[i], "-i") == 0) {
				opts->interactive = 1;
			} else if (cmp(argv[i], "--verbose") == 0 || cmp(argv[i], "-v") == 0) {
				opts->verbose = 1;
			} else if (cmp(argv[i], "--recursive") == 0 || cmp(argv[i], "-r") == 0 || cmp(argv[i], "-R") == 0) {
				opts->recursive = 1;
			} else if (cmp(argv[i], "--preserve") == 0 || cmp(argv[i], "-p") == 0) {
				opts->preserve = 1;
			} else if (cmp(argv[i], "--no-dereference") == 0 || cmp(argv[i], "-P") == 0) {
				opts->no_dereference = 1;
			} else if (cmp(argv[i], "--update") == 0 || cmp(argv[i], "-u") == 0) {
				opts->update = 1;
			} else if (cmp(argv[i], "--backup") == 0) {
				opts->backup = 1;
			} else if (argv[i][1] == '-') {
				const char *msg = "mini-cp: unrecognized option '";
				write(STDERR_FILENO, msg, len(msg));
				write(STDERR_FILENO, argv[i], len(argv[i]));
				write(STDERR_FILENO, "'\n", 2);
				show_help();
				exit_asm(1);
			} else {
				for (int j = 1;argv[i][j] != '\0';j++) {
					switch (argv[i][j]) {
						case 'f': opts->force = 1; break;
						case 'i': opts->interactive = 1; break;
						case 'v': opts->verbose = 1; break;
						case 'r': case 'R': opts->recursive = 1; break;
						case 'p': opts->preserve = 1; break;
						case 'P': opts->no_dereference = 1; break;
						case 'u': opts->update = 1; break;
						default:
							const char *msg = "cp: invalid option -- '";
							write(STDERR_FILENO, msg, len(msg));
							char c[2] = {argv[i][j], '\0'};
							write(STDERR_FILENO, c, 1);
							write(STDERR_FILENO, "'\n", 2);
							show_help();
							exit_asm(1);
					}
				}
			}
		} else {
			break;
		}
	}
}

void c_start(long *sp)
{
	int argc = (int)*sp;
	char **argv = (char **)(sp + 1);

	if (argc < 2) {
		show_help();
		exit_asm(1);
	}

	cp_options_t opts = {0};
	cp_stats_t stats = {0};

	parse_args(argc, argv, &opts);

	int first_arg = 1;
	while (first_arg < argc && argv[first_arg][0] == '-') first_arg++;

	int num_sources = argc - first_arg - 1;

	if (num_sources < 1) {
		const char *msg = "mini-cp: missing file operand\n";
		write(STDERR_FILENO, msg, len(msg));
		show_help();
		exit_asm(1);
	}

	const char* dest = argv[argc - 1];

	if (num_sources > 1) {
		if (!is_directory(dest)) {
			const char *msg = "mini-cp: target '";
			write(STDERR_FILENO, msg, len(msg));
			write(STDERR_FILENO, dest, len(dest));
			write(STDERR_FILENO, "' is not a directory\n", 21);
			exit_asm(1);
		}
	}

	for (int i = first_arg;i < argc - 1;i++) {
		const char* src = argv[i];

		char dst_path[PATH_MAX];
		if (num_sources > 1) {
			scpy(dst_path, dest);
			if (dst_path[len(dst_path)-1] != '/') scat(dst_path, "/");

			const char* filename = src;
			for (const char *p = src;*p != '\0';p++) {
				if (*p == '/') filename = p + 1;
			}
			scat(dst_path, filename);
		} else {
			scpy(dst_path, dest);
		}

		process_item(src, dst_path, &opts, &stats);
	}

	if (opts.verbose) {
		char summary[256];
		scpy(summary, "\nSummary: ");

		char num_str[32];

		scat(summary, "files=");
		char *p = num_str + 31;
		*p = '\0';
		uint64_t n = stats.files_copied;
		do {
			*--p = '0' + (n % 10);
			n /= 10;
		} while (n > 0);
		scat(summary, p);

		scat(summary, ", dirs=");
		p = num_str + 31;
		*p = '\0';
		n = stats.directories_created;
		do {
			*--p = '0' + (n % 10);
			n /= 10;
		} while (n > 0);
		scat(summary, p);

		scat(summary, ", bytes=");
		p = num_str + 31;
		*p = '\0';
		n = stats.bytes_copied;

		do {
			*--p = '0' + (n % 10);
			n /= 10;
		} while (n > 0);
		scat(summary, p);

		scat(summary, ", errors=");
		p = num_str + 31;
		*p = '\0';
		n = stats.errors;
		do {
			*--p = '0' + (n % 10);
			n /= 10;
		} while (n > 0);
		scat(summary, p);

		scat(summary, "\n");
		write(STDOUT_FILENO, summary, len(summary));
	}

	exit_asm(stats.errors > 0 ? 1 : 0);
}

void __attribute__((naked)) _start(void)
{
	asm volatile (
		"mov %rsp, %rdi\n"
		"andq $-16, %rsp\n"
		"call c_start"
	);
}
