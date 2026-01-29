#ifndef LIB_H
#define LIB_H

#include "sys/guicall.h"

/*
 * Note: int64_t and uint64_t
 */

typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef int64_t off64_t;
typedef uint64_t ino64_t;

#ifndef NULL
#define NULL ((void *)0)
#endif

#define O_RDONLY 0000000
#define O_DIRECTORY 0200000
#define PROT_READ 0x1
#define PROT_WRITE 0x2
#define MAP_PRIVATE 0x02
#define MAP_ANONYMOUS 0x20
#define MAP_FAILED ((void *) -1)

#if !defined(STDIN_FILENO) && !defined(STDOUT_FILENO) && !defined(STDERR_FILENO)
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
#endif

#define len(str) \
({ \
 	long ret; \
	unsigned long c = 0; \
	while (str[c] != '\0') ++c; \
	ret = (long)c; \
	ret; \
})

#define nlen(str, maxlen) \
({ \
 	long ret; \
	if (!str) { \
		ret = (long)NULL; \
	} else { \
		unsigned long l = 0; \
		while (l < maxlen && str[l] != '\0') ++l; \
		ret = (long)l; \
	} \
	ret; \
})

long write(int fd, const char* msg, unsigned long msg_len);
char *scpy(char* dest, const char* src);
char *scat(char* dest, const char* src);
int cmp(const char* s1, const char* s2);

#endif
