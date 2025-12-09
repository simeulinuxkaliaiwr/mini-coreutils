#define _GNU_SOURCE

#include "lib.h"
#include <linux/limits.h>
#include <sys/syscall.h>
#include <unistd.h>

static const char *get_env_var(const char *name) {
    extern char **environ;
    
    if (environ == NULL) {
        return NULL;
    }
    
    size_t name_len = guilen(name);
    
    for (char **env = environ; *env != NULL; env++) {
        const char *env_str = *env;
        size_t i;
        
        for (i = 0; i < name_len; i++) {
            if (env_str[i] != name[i]) {
                break;
            }
        }
        
        if (i == name_len && env_str[i] == '=') {
            return &env_str[i + 1];
        }
    }
    
    return NULL;
}

/**
 * @brief Verifies if the path is absolute. (Starts with '/')
 */

static int is_absolute_path(const char *path) {
    return path != NULL && path[0] == '/';
}

int main(int argc, char *argv[]) {
    int logical = 1;
    int show_physical = 0;
    
    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];
        
        if (guicmp(arg, "-L") == 0) {
            logical = 1;
        } else if (guicmp(arg, "-P") == 0) {
            logical = 0;
            show_physical = 1;
        } else if (guicmp(arg, "--help") == 0) {
            const char *help_msg = 
                "mini-pwd (low-level version)\n"
                "Usage: mini-pwd [-L|-P]\n"
                "  -L  Use PWD from environment (default)\n"
                "  -P  Avoid symlinks (physical path)\n";
            syscall(SYS_write, STDOUT_FILENO, help_msg, guilen(help_msg));
            syscall(SYS_exit, 0);
        } else if (arg[0] == '-') {
            const char *err = "mini-pwd: invalid option\n";
            syscall(SYS_write, STDERR_FILENO, err, guilen(err));
            syscall(SYS_exit, 1);
        }
    }
    
    char cwd_buffer[PATH_MAX];
    
    if (logical) {
        const char *pwd_value = get_env_var("PWD");
        
        if (pwd_value != NULL && is_absolute_path(pwd_value)) {
            syscall(SYS_write, STDOUT_FILENO, pwd_value, guilen(pwd_value));
            syscall(SYS_write, STDOUT_FILENO, "\n", 1);
            syscall(SYS_exit, 0);
        }
    }
    
    long ret = syscall(SYS_getcwd, cwd_buffer, sizeof(cwd_buffer));
    
    if (ret < 0) {
        const char *prefix = "mini-pwd: ";
        syscall(SYS_write, STDERR_FILENO, prefix, guilen(prefix));
        gui_perror(NULL);
        syscall(SYS_exit, 1);
    }
    
    syscall(SYS_write, STDOUT_FILENO, cwd_buffer, guilen(cwd_buffer));
    syscall(SYS_write, STDOUT_FILENO, "\n", 1);
    
    syscall(SYS_exit, 0);
}
