#ifndef LIB_H
#define LIB_H

#if !defined(__x86_64__)
#error "This project only supports Linux x86_64"
#endif

#include "sys/guicall.h"
#include "sys/errno.h"

/*
 * Note: int64_t and uint64_t are defined in sys/guicall.h.
 */

typedef signed int int32_t;
typedef unsigned int uint32_t;

typedef int64_t off64_t;
typedef uint64_t ino64_t;

extern int errno;

#define ALIGNMENT 16
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define MIN_BLOCK_SIZE (ALIGNMENT * 2)
#define PAGE_SIZE 4096

typedef struct block_header {
	unsigned long size;
	int free;
	struct block_header *next;
} block_header_t;

#define HEADER_SIZE sizeof(block_header_t)
extern block_header_t *free_list;

#ifndef NULL
#define NULL ((void *)0)
#endif

#define S_IFMT   0170000
#define S_IFDIR  0040000
#define S_IFREG  0100000
#define S_IFLNK  0120000

#define F_OK 0
#define W_OK 2

#define AT_FDCWD -100

struct timespec {
    long tv_sec;
    long tv_nsec;
};

#define O_RDONLY 0000000
#define O_WRONLY 01
#ifndef O_CREAT
#define O_CREAT 0100
#endif
#ifndef O_TRUNC
#define O_TRUNC 01000
#endif
#define O_DIRECTORY 0200000
#define PROT_READ 0x1
#define PROT_WRITE 0x2
#define MAP_PRIVATE 0x02
#define MAP_ANONYMOUS 0x20
#define MAP_FAILED ((void *) -1)

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define len(str) \
({ \
 	long ret; \
	unsigned long c = 0; \
	while (str[c] != '\0') ++c; \
	ret = (long)c; \
	ret; \
})

void __attribute__((noreturn)) exit_asm(int code);
long write(int fd, const char* msg, unsigned long msg_len);
char *scpy(char* dest, const char* src);
char *scat(char* dest, const char* src);
int cmp(const char* s1, const char* s2);
const char *strerror(int errnum);

void *malloc(unsigned long size);
void free(void *ptr);
void *calloc(unsigned long nmemb, unsigned long size);
void *realloc(void *ptr, unsigned long size);

#endif
