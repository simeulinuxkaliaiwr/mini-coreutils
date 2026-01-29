#include "lib.h"

void cat(const char* filename)
{
	int fd = guicall(SYS_open, filename, O_RDONLY);
	if (fd < 0) {
		const char* prefix = "\033[31mmini-cat: ";
		const char* sufix = ": no such file or directory\033[0m\n";
		write(STDERR_FILENO, prefix, len(prefix));
		write(STDERR_FILENO, filename, len(filename));
		write(STDERR_FILENO, sufix, len(sufix));
		guicall(SYS_exit, 1);
	}
	char buffer[(1024 * 8)];
	long bytes_read;
	while ((bytes_read = guicall(SYS_read, fd, buffer, sizeof(buffer))) > 0) {
		write(STDOUT_FILENO, buffer, bytes_read);
	}
	guicall(SYS_close, fd);
}

void cat_stdin(void)
{
	char buffer[1024];
	long bytes_read;
	while ((bytes_read = guicall(SYS_read, STDIN_FILENO, buffer, sizeof(buffer))) > 0) {
		write(STDOUT_FILENO, buffer, bytes_read);
	}
}

void c_start(long *sp)
{
	int argc = (int)*sp;
	char **argv = (char **)(sp + 1);

	if (argc == 1) {
		cat_stdin();
	} else {
		for (int i = 1;i < argc;i++) {
			cat(argv[i]);
		}
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
