#define _GNU_SOURCE

#include <linux/fcntl.h>
#include <sys/syscall.h>
#include <unistd.h>

size_t guilen(const char *str) {
  size_t counter = 0;
  while (str[counter] != '\0') {
    ++counter;
  }
  return counter;
}

void error(const char *msg) {
  syscall(SYS_write, STDOUT_FILENO, msg, guilen(msg));
}

void cat(const char *pathname) {
  char buffer[(1024 * 8)];
  int fd = syscall(SYS_open, pathname, O_RDONLY);
  if (fd < 0) {
    error("Mini-cat: ");
    error(pathname);
    error(": No such file or directory.\n");
    syscall(SYS_exit, 1);
  }
  ssize_t bytes_read;
  while ((bytes_read = syscall(SYS_read, fd, buffer, sizeof(buffer))) > 0) {
    ssize_t bytes_written = 0;
    while (bytes_written < bytes_read) {
      ssize_t result = syscall(SYS_write, STDOUT_FILENO, buffer + bytes_written,
                               bytes_read - bytes_written);
      if (result < 0) {
        error("Error writing in stdout.\n");
        syscall(SYS_close, fd);
        syscall(SYS_exit, 1);
      }
      bytes_written += result;
    }
  }
  if (bytes_read < 0) {
    error("Error reading file!\n");
  }
  syscall(SYS_close, fd);
}

void cat_stdin(void) {
  char buffer[(1024 * 8)];
  ssize_t bytes_read;
  while ((bytes_read =
              syscall(SYS_read, STDIN_FILENO, buffer, sizeof(buffer))) > 0) {
    ssize_t bytes_written = 0;
    while (bytes_written < bytes_read) {
      ssize_t result = syscall(SYS_write, STDOUT_FILENO, buffer + bytes_written,
                               bytes_read - bytes_written);
      if (result < 0) {
        error("Error writing in stdout.\n");
        syscall(SYS_exit, 1);
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
  syscall(SYS_exit, 0);
}
