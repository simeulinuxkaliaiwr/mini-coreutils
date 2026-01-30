#if !defined(__x86_64__)
#error "This project only supports Linux x86_64"
#endif

#include "lib.h"

#define PATH_MAX 1024

static const char* get_env_var(const char* name, char **envp)
{
	if (envp == NULL) return NULL;

	unsigned long name_len = len(name);

	for (char **env = envp; *env != NULL; env++) {
		const char* env_str = *env;
		unsigned long i;

		// Compara o nome da variável (ex: PWD)
		for (i = 0; i < name_len; i++) {
			if (env_str[i] != name[i]) {
				break;
			}
		}

		// Se encontrou o nome seguido de '=', retorna o valor
		if (i == name_len && env_str[i] == '=') {
			return &env_str[i + 1];
		}
	}

	return NULL;
}

/**
 * @brief Verifie if the path is absolute. (Starts with '/')
 */

static int is_absolute_path(const char* path) {
	return path != NULL && path[0] == '/';
}

void c_start(long *sp)
{
	int argc = (int)*sp;
	char **argv = (char **)(sp + 1);
	
	char **envp = (char **)(sp + argc + 2);


	int logical = 1;
	int show_physical = 0;

	for (int i = 1;i < argc;i++) {
		const char* arg = argv[i];

		if (cmp(arg, "-L") == 0) {
			logical = 1;
		} else if (cmp(arg, "-P") == 0) {
			logical = 0; show_physical = 1;
		} else if (cmp(arg, "--help") == 0 || cmp(arg, "-h") == 0) {
			const char* help_msg =
				"mini-pwd (low-level)\n"
				"Usage: mini-pwd [-L|-P]\n"
				"  -L	Use PWD from environment (default)\n"
				"  -P	Avoid syslinks (physical path)\n";
			write(STDOUT_FILENO, help_msg, len(help_msg));
			guicall(SYS_exit, 0);
		} else if (arg[0] == '-') {
			const char* err = "mini-pwd: invalid option\n";
			write(STDERR_FILENO, err, len(err));
			guicall(SYS_exit, 1);
		}
	}

	char cwd_buffer[PATH_MAX];

	if (logical) {
		const char* pwd_value = get_env_var("PWD", envp);

		if (pwd_value != NULL && is_absolute_path(pwd_value)) {
			write(STDOUT_FILENO, pwd_value, len(pwd_value));
			write(STDOUT_FILENO, "\n", 1);
			guicall(SYS_exit, 0);
		}
	}

	long ret = guicall(SYS_getcwd, cwd_buffer, sizeof(cwd_buffer));

	if (ret < 0) {
		const char* msg = "mini-pwd: getcwd syscall failed.\n";
		write(STDERR_FILENO, msg, len(msg));
		guicall(SYS_exit, 0);
	}

	write(STDOUT_FILENO, cwd_buffer, len(cwd_buffer));
	write(STDOUT_FILENO, "\n", 1);

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
