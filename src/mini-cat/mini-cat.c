#define _GNU_SOURCE

#include "lib.h"
#include <linux/fcntl.h>
#include "sys/guicall.h"
#include "sys/sysnums.h"

void error(const char *msg) {
  guicall(SYS_write, STDERR_FILENO, (int64_t)msg, guilen(msg));
}

void cat(const char *pathname) {
  char buffer[(1024 * 8)];
  int fd = guicall(SYS_open, (int64_t)pathname, O_RDONLY);
  if (fd < 0) {
    error("Mini-cat: ");
    error(pathname);
    error(": No such file or directory.\n");
    guicall(SYS_exit, 1, 0, 0, 0, 0, 0);
  }
  ssize_t bytes_read;
  while ((bytes_read = guicall(SYS_read, fd, (int64_t)buffer, sizeof(buffer))) > 0) {
    ssize_t bytes_written = 0;
    while (bytes_written < bytes_read) {
      ssize_t result = guicall(SYS_write, STDOUT_FILENO, (int64_t)(buffer + bytes_written),
                               bytes_read - bytes_written);
      if (result < 0) {
        error("Error writing in stdout.\n");
        guicall(SYS_close, fd, 0, 0, 0, 0, 0);
        guicall(SYS_exit, 1, 0, 0, 0, 0, 0);
      }
      bytes_written += result;
    }
  }
  if (bytes_read < 0) {
    error("Error reading file!\n");
  }
  guicall(SYS_close, fd, 0, 0, 0, 0, 0);
}

void cat_stdin(void) {
  char buffer[(1024 * 8)];
  ssize_t bytes_read;
  while ((bytes_read =
              guicall(SYS_read, STDIN_FILENO, (int64_t)buffer, sizeof(buffer))) > 0) {
    ssize_t bytes_written = 0;
    while (bytes_written < bytes_read) {
      ssize_t result = guicall(SYS_write, STDOUT_FILENO, (int64_t)(buffer + bytes_written),
                               bytes_read - bytes_written);
      if (result < 0) {
        error("Error writing in stdout.\n");
        guicall(SYS_exit, 1, 0, 0, 0, 0, 0);
      }
      bytes_written += result;
    }
  }
}

int main(int argc, const char *argv[]) {
  if (argc == 1) {
    cat_stdin();
  } else {
    for (int i = 1; i < argc; ++i) {
      cat(argv[i]);
    }
  }
  guicall(SYS_exit, 0, 0, 0, 0, 0, 0);
}
