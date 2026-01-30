#if !defined(__x86_64__)
#error "This project only supports Linux x86_64"
#endif

#include "lib.h"

int cat(const char* filename)
{
	int fd = guicall(SYS_open, filename, O_RDONLY);
	if (fd < 0) {
		const char* prefix = "\033[31mmini-cat: ";
		const char* sufix = ": no such file or directory\033[0m\n";
		write(STDERR_FILENO, prefix, len(prefix));
		write(STDERR_FILENO, filename, len(filename));
		write(STDERR_FILENO, sufix, len(sufix));
		return -1;
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
		for (int i = 1; i < argc; i++) {
		// Se tivermos múltiplos arquivos, imprimimos um separador visual
			if (argc > 2) {
				const char* header_color = "\033[1;34m==> "; // Azul Negrito
				const char* header_end = " <==\033[0m\n";
                
				write(STDOUT_FILENO, header_color, len(header_color));
				write(STDOUT_FILENO, argv[i], len(argv[i]));
				write(STDOUT_FILENO, header_end, len(header_end));
			}

			if (cmp(argv[i], "-") == 0) {
				cat_stdin();
			} else {
				cat(argv[i]);
			}

			if (argc > 2 && i < argc - 1) {
				write(STDOUT_FILENO, "\n", 1);
			}
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
