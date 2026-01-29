#include "lib.h"

long write(int fd, const char* msg, unsigned long msg_len)
{
	long ret;
	asm volatile (
		"syscall"
		: "=a" (ret)
		: "a" (SYS_write), "D" (fd), "S" (msg), "d" (msg_len)
		: "rcx", "r11", "memory", "cc"
	);
	if (ret <= 0) {
		const char* file_desc;
		switch(fd) {
			case 1: file_desc = "STDOUT_FILENO"; break;
			case 2: file_desc = "STDERR_FILENO"; break;
			default: file_desc = "UNKNOWN_FD"; break;
		}
		const char* prefix = "\033[31m[FATAL] Cannot write to ";
		const char* sufix = "\033[0m\n";
		asm volatile ("syscall" : : "a" (SYS_write), "D" (STDERR_FILENO), "S" (prefix), "d" (len(prefix)) : "rcx", "r11", "memory");
		asm volatile ("syscall" : : "a" (SYS_write), "D" (STDERR_FILENO), "S" (file_desc), "d" (len(file_desc)) : "rcx", "r11", "memory");
		asm volatile ("syscall" : : "a" (SYS_write), "D" (STDERR_FILENO), "S" (sufix), "d" (len(sufix)) : "rcx", "r11", "memory");
		
		asm volatile (
			"syscall"
			:
			: "a" (SYS_exit), "D" (1)
			: "rcx", "r11", "memory"
		);
	}
	return ret;
}

char *scpy(char* dest, const char* src)
{
	char* orig_dest = dest;
	while ((*dest++ = *src++)) {
		// No need for nothing here.
	}
	return orig_dest;
}

char *scat(char* dest, const char* src)
{
	char* orig_dest = dest;
	while (*dest != '\0') {
		++dest;
	}
	scpy(dest, src);
	return orig_dest;
}

int cmp(const char* s1, const char* s2)
{
	while (*s1 && (*s1 == *s2)) {
		++s1;
		++s2;
	}
	return (int)*((const unsigned char *)s1) - (int)*((const unsigned char *)s2);
}
