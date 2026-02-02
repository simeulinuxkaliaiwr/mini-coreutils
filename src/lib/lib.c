#if !defined(__x86_64__)
#error "This project only supports linux x86_64"
#endif

#include "lib.h"

int errno = 0;

void __attribute__((noreturn)) exit_asm(int code)
{
	asm volatile (
		"syscall"
		:
		: "a" (SYS_exit), "d" (code)
		: "rcx", "r11", "memory"
	);
	__builtin_unreachable();
}

long write(int fd, const char* msg, unsigned long msg_len) {
	long ret;
	asm volatile ("syscall" : "=a"(ret) : "a"(SYS_write), "D"(fd), "S"(msg), "d"(msg_len) : "rcx", "r11", "memory");

	if (ret < 0) {
		const char *err = "[FATAL] Write Error\n";
		asm volatile ("syscall" : : "a"(SYS_write), "D"(STDERR_FILENO), "S"(err), "d"(20) : "rcx", "r11", "memory");
		/* Removed the exit syscall */
	}
	return ret;
}

const char* strerror(int errnum)
{
	switch(errnum) {
		case EPERM:  return "Operation not permitted";
		case ENOENT: return "No such file or directory";
		case EACCES: return "Permission denied";
		case ENOMEM: return "Out of memory";
		case EINVAL: return "Invalid argument";
		case EEXIST: return "File exists";
		case EBADF:  return "Bad file descriptor";
		default:     return "Unknown error";
	}
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
		s1++; s2++;
	}
	return *(unsigned char*)s1 - *(unsigned char*)s2;
}

block_header_t *free_list = NULL;

void *malloc(unsigned long size)
{
	if (size == 0) return NULL;

	size = ALIGN(size);
	if (size == 0) return NULL;

	block_header_t *current = free_list;
	block_header_t *prev = NULL;
	block_header_t *best_fit = NULL;
	block_header_t *best_fit_prev = NULL;

	while (current) {
		if (current->free && current->size >= size) {
			if (!best_fit || current->size < best_fit->size) {
				best_fit = current;
				best_fit_prev = prev;
			}
		}

		prev = current;
		current = current->next;
	}

	if (best_fit) {
		unsigned long remaining = best_fit->size - size;
		if (remaining >= HEADER_SIZE + MIN_BLOCK_SIZE) {
			block_header_t *new_block = (block_header_t *)((char *)(best_fit + 1) + size);
			new_block->size = remaining - HEADER_SIZE;
			new_block->free = 1;
			new_block->next = best_fit->next;

			best_fit->size = size;
			best_fit->next = new_block;
		}

		best_fit->free = 0;
		return (void *)(best_fit + 1);
	}

	unsigned long total_size = size + HEADER_SIZE;
	unsigned long pages = (total_size + PAGE_SIZE - 1) / PAGE_SIZE;
	unsigned long alloc_size = pages * PAGE_SIZE;

	block_header_t *block = (block_header_t *)guicall(SYS_mmap, NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

	if (block == MAP_FAILED) {
		errno = ENOMEM;
		return NULL;
	}

	block->size = alloc_size - HEADER_SIZE;
	block->free = 0;
	block->next = NULL;

	if (!free_list || block < free_list) {
		block->next = free_list;
		free_list = block;
	} else {
		block_header_t *cur = free_list;
		while (cur->next && cur->next < block) {
			cur = cur->next;
		}
		block->next = cur->next;
		cur->next = block;
	}

	if (block->size > size + HEADER_SIZE + MIN_BLOCK_SIZE) {
		block_header_t *new_block = (block_header_t *)((char *)(block + 1) + size);
		new_block->size = block->size - size - HEADER_SIZE;
		new_block->free = 1;
		new_block->next = block->next;

		block->size = size;
		block->next = new_block;
	}

	block->free = 0;
	return (void *)(block + 1);
}

void free(void *ptr)
{
	if (!ptr) return;

	block_header_t *header = (block_header_t *)ptr - 1;
	header->free = 1;

	block_header_t *cur = free_list;
	block_header_t *prev = NULL;

	while (cur) {
		if (cur->free && cur->next && cur->next->free) {
			char *cur_end = (char *)(cur + 1) + cur->size;
			if (cur_end == (char *)cur->next) {
					cur->size += HEADER_SIZE + cur->next->size;
					cur->next = cur->next->next;
					continue; // Verifica novamente
			}
		}

        // Coalescência com anterior
		if (prev && prev->free && cur->free) {
			char *prev_end = (char *)(prev + 1) + prev->size;
			if (prev_end == (char *)cur) {
				prev->size += HEADER_SIZE + cur->size;
				prev->next = cur->next;
				cur = prev; // Volta para verificar novamente
			}
		}

		prev = cur;
		cur = cur->next;
	}

	cur = free_list;
	prev = NULL;
	while (cur) {
		if (cur->free) {
			unsigned long block_size = cur->size + HEADER_SIZE;
			if (block_size >= PAGE_SIZE && (block_size % PAGE_SIZE) == 0) {
                // Pode devolver ao sistema com munmap
                // Mas cuidado com fragmentação!
				if (prev) prev->next = cur->next;
				else free_list = cur->next;

				guicall(SYS_munmap, cur, block_size);
				break;
			}
		}
		prev = cur;
		cur = cur->next;
	}
}

void *realloc(void *ptr, unsigned long size)
{
	if (!ptr) return malloc(size);

	if (size == 0) {
		free(ptr);
		return NULL;
	}

	size = ALIGN(size);
	block_header_t *header = (block_header_t *)ptr - 1;

	if (header->size >= size) {
		unsigned long remaining = header->size - size;
		if (remaining >= HEADER_SIZE + MIN_BLOCK_SIZE) {
			block_header_t *new_block = (block_header_t *)((char *)(header + 1) + size);
			new_block->size = remaining - HEADER_SIZE;
			new_block->free = 1;
			new_block->next = header->next;

			header->size = size;
			header->next = new_block;
			free((void *)(new_block + 1)); // Ativa coalescência
		}
		return ptr;
	}
	if (header->next && header->next->free) {
		char *header_end = (char *)(header + 1) + header->size;
		if (header_end == (char *)header->next) {
			unsigned long total_size = header->size + HEADER_SIZE + header->next->size;
			if (total_size >= size) {
				header->next = header->next->next;
				header->size = total_size - HEADER_SIZE;
				return ptr;
			}
		}
	}

	void *new_ptr = malloc(size);
	if (!new_ptr) return NULL;

	unsigned long copy_size = header->size < size ? header->size : size;
	unsigned char *src = (unsigned char *)ptr;
	unsigned char *dst = (unsigned char *)new_ptr;

	for (unsigned long i = 0; i < copy_size; i++) {
		dst[i] = src[i];
    }

    free(ptr);
    return new_ptr;
}

void *calloc(unsigned long nmemb, unsigned long size)
{
	if (nmemb == 0 || size == 0) return NULL;

	unsigned long total = nmemb * size;
	if (nmemb != 0 && total / nmemb != size) {
		errno = ENOMEM;
		return NULL;
	}

	void *ptr = malloc(total);
	if (ptr) {
		unsigned char *p = (unsigned char *)ptr;
		for (unsigned long i = 0;i < total;i++) {
			p[i] = 0;
		}
	}
	return ptr;
}
