#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64

#include "lib.h"
#include "sys/guicall.h"    
#include "sys/sysnums.h"    
#include <getopt.h>
#include <linux/fcntl.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>

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
  int64_t st_size;
  int64_t st_blksize;
  int64_t st_blocks;
  uint64_t st_atime;
  uint64_t st_atime_nsec;
  uint64_t st_mtime;
  uint64_t st_mtime_nsec;
  uint64_t st_ctime;
  uint64_t st_ctime_nsec;
  int64_t __unused[3];
};

typedef struct {
  int recursive;
  int all;
  int long_format;
} Options;

void show_help() {
  const char *msg =
      "Mini-LS made by me\n"
      "Use: mini-ls [OPTIONS] [PATH]\n\n"
      "Opções:\n"
      "  -l, --long        use a long listing format (mode, uid, "
      "gid, size, etc)\n"
      "  -a, --all         do not ignore entries starting with .\n"
      "  -r, --recursive   list subdirectories recursively\n\n"
      "Example:\n"
      "  ./a.out -la /etc\n"
      "  ./a.out -r ~\n";

  guicall(SYS_write, STDOUT_FILENO, msg, guilen(msg));
  guicall(SYS_exit, 0);
}

void write_num(int64_t num) {
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

  for (int j = i - 1; j >= 0; --j) {
    guicall(SYS_write, STDOUT_FILENO, &buf[j], 1);
  }
}

void write_mode(uint32_t mode) {
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

  guicall(SYS_write, STDOUT_FILENO, perms, 10);
}

void show_long(const char *path, const char *name) {
  char full[4096];
  guicpy(full, path);
  if (full[guilen(full) - 1] != '/') {
    guicat(full, "/");
  }
  guicat(full, name);

  struct stat64 info;
  if (guicall(SYS_stat, full, &info) < 0) {
    return;
  }

  write_mode(info.st_mode);
  guicall(SYS_write, STDOUT_FILENO, " ", 1);
  write_num(info.st_nlink);
  guicall(SYS_write, STDOUT_FILENO, " ", 1);
  write_num(info.st_uid);
  guicall(SYS_write, STDOUT_FILENO, " ", 1);
  write_num(info.st_gid);
  guicall(SYS_write, STDOUT_FILENO, " ", 1);
  write_num(info.st_size);
  guicall(SYS_write, STDOUT_FILENO, " ", 1);
  guicall(SYS_write, STDOUT_FILENO, name, guilen(name));

  if ((info.st_mode & 0170000) == 0120000) {
    char target[4096];
    int len = guicall(SYS_readlink, full, target, sizeof(target) - 1);
    if (len > 0) {
      target[len] = '\0';
      guicall(SYS_write, STDOUT_FILENO, " -> ", 4);
      guicall(SYS_write, STDOUT_FILENO, target, len);
    }
  }

  guicall(SYS_write, STDOUT_FILENO, "\n", 1);
}

void list(const char *path, Options *opt);

void scan_recursive(const char *path, Options *opt) {
  int fd = guicall(SYS_open, path, O_RDONLY | O_DIRECTORY);
  if (fd < 0) {
    return;
  }

  size_t buf_size = (1024 * 32);
  char *buf = (char *)guicall(SYS_mmap, NULL, buf_size, PROT_READ | PROT_WRITE,
                              MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  if (buf == MAP_FAILED) {
    guicall(SYS_close, fd);
    return;
  }

  int nread;
  while ((nread = guicall(SYS_getdents64, fd, buf, buf_size)) > 0) {
    for (size_t bpos = 0; bpos < (size_t)nread;) {
      struct linux_dirent64 *d = (void *)(buf + bpos);
      if (guicmp(d->d_name, ".") != 0 && guicmp(d->d_name, "..") != 0) {
        if (d->d_type == 4) {
          char subpath[4096];
          guicpy(subpath, path);
          if (subpath[guilen(subpath) - 1] != '/') {
            guicat(subpath, "/");
          }
          guicat(subpath, d->d_name);

          guicall(SYS_write, STDOUT_FILENO, "\n", 1);
          guicall(SYS_write, STDOUT_FILENO, subpath, guilen(subpath));
          guicall(SYS_write, STDOUT_FILENO, ":\n", 2);
          list(subpath, opt);
        }
      }
      bpos += d->d_reclen;
    }
  }

  guicall(SYS_close, fd);
  guicall(SYS_munmap, buf, buf_size);
}

void list(const char *path, Options *opt) {
  int fd = guicall(SYS_open, path, O_RDONLY | O_DIRECTORY);
  if (fd < 0) {
    const char *msg = "Open syscall failed.\n";
    guicall(SYS_write, STDERR_FILENO, msg, guilen(msg));
    guicall(SYS_exit, 1);
  }

  size_t buf_size = (1024 * 32);
  char *buf = (char *)guicall(SYS_mmap, NULL, buf_size, PROT_READ | PROT_WRITE,
                              MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
  if (buf == MAP_FAILED) {
    const char *msg = "MMap syscall failed.\n";
    guicall(SYS_write, STDERR_FILENO, msg, guilen(msg));
    guicall(SYS_close, fd);
    guicall(SYS_exit, 1);
  }

  int nread;
  while ((nread = guicall(SYS_getdents64, fd, buf, buf_size)) > 0) {
    for (size_t bpos = 0; bpos < (size_t)nread;) {
      struct linux_dirent64 *d = (void *)(buf + bpos);
      int hidden = d->d_name[0] == '.';

      if (!opt->all && hidden) {
        bpos += d->d_reclen;
        continue;
      }

      if (guicmp(d->d_name, ".") != 0 && guicmp(d->d_name, "..") != 0) {
        if (opt->long_format) {
          show_long(path, d->d_name);
        } else {
          guicall(SYS_write, STDOUT_FILENO, d->d_name, guilen(d->d_name));
          if (d->d_type == 4) {
            guicall(SYS_write, STDOUT_FILENO, "/", 1);
          }
          guicall(SYS_write, STDOUT_FILENO, "\n", 1);
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

int main(int argc, char *argv[]) {
  Options opt = {0, 0, 0};

  static struct option long_opts[] = {{"recursive", no_argument, 0, 'r'},
                                      {"all", no_argument, 0, 'a'},
                                      {"long", no_argument, 0, 'l'},
                                      {"help", no_argument, NULL, 'h'},
                                      {0, 0, 0, 0}};

  int c;
  while ((c = getopt_long(argc, argv, "rahl", long_opts, NULL)) != -1) {
    switch (c) {
    case 'r':
      opt.recursive = 1;
      break;
    case 'h':
      show_help();
      break;
    case 'a':
      opt.all = 1;
      break;
    case 'l':
      opt.long_format = 1;
      break;
    }
  }

  if (optind == argc) {
    list(".", &opt);
  } else {
    for (int i = optind; i < argc; ++i) {
      if (argc - optind > 1) {
        guicall(SYS_write, STDOUT_FILENO, argv[i], guilen(argv[i]));
        guicall(SYS_write, STDOUT_FILENO, ":", 1);
        guicall(SYS_write, STDOUT_FILENO, "\n", 1);
      }
      list(argv[i], &opt);
      if (i < argc - 1 && argc - optind > 1) {
        guicall(SYS_write, STDOUT_FILENO, "\n", 1);
      }
    }
  }
  guicall(SYS_exit, 0);
}
