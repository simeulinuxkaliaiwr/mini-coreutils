#define _GNU_SOURCE

#include "lib.h"
#include <sys/syscall.h>

int main(int argc, const char *argv[]) {
  int i;
  int no_newline = 0;
  if (argc > 1 && guicmp(argv[1], "-n") == 0) {
    no_newline = 1;
    i = 2;
  } else {
    i = 1;
  }
  for (; i < argc; ++i) {
    syscall(SYS_write, STDOUT_FILENO, argv[i], guilen(argv[i]));
    if (i < argc - 1) {
      syscall(SYS_write, STDOUT_FILENO, " ", 1);
    }
  }
  if (!no_newline) {
    syscall(SYS_write, STDOUT_FILENO, "\n", 1);
  }
  syscall(SYS_exit, 0);
}
