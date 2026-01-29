#include "lib.h"

void c_start(long *sp)
{
	int argc = (int)*sp;
	char **argv = (char **)(sp + 1);

	int i;
	int no_newline = 0;
	if (argc > 1 && cmp(argv[1], "-n") == 0) {
		no_newline = 1;
		i = 2;
	} else {
		i = 1;
	}

	for (; i < argc;i++) {
		write(STDOUT_FILENO, argv[i], len(argv[i]));
		if (i < argc - 1) {
			write(STDOUT_FILENO, " ", 1);
		}
	}

	if (!no_newline) {
		write(STDOUT_FILENO, "\n", 1);
	}
	guicall(SYS_exit, 0);
}

void __attribute__((naked)) _start()
{
	asm volatile (
		"mov %rsp, %rdi\n"
		"andq $-16, %rsp\n"
		"call c_start"
	);
}
