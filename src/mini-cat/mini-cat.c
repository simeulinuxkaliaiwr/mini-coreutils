#if !defined(__x86_64__)
#error "This project only supports Linux x86_64"
#endif

#include "lib.h"

int cat(const char* filename)
{
	int fd = guicall(SYS_open, filename, O_RDONLY);
	if (fd < 0) {
		int err = extract_error(fd); // Converte o retorno negativo do syscall
		const char* msg_err = strerror(err);

		write(STDERR_FILENO, "cat: ", 5);
		write(STDERR_FILENO, filename, len(filename));
		write(STDERR_FILENO, ": ", 2);
		write(STDERR_FILENO, msg_err, len(msg_err));
		write(STDERR_FILENO, "\n", 1);
		return -1;
	}
	
	struct stat64 st;
	if (guicall(SYS_fstat, fd, &st) < 0) {
		guicall(SYS_close, fd);
		return -1;
	}

	off64_t offset = 0;
	long total_sent = 0;

	while (total_sent < st.st_size) {
		long ret = guicall(SYS_sendfile, STDIN_FILENO, fd, &offset, st.st_size - total_sent);
		if (ret <= 0) break;
		total_sent += ret;
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
