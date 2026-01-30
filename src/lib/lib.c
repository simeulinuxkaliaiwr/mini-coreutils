#if !defined(__x86_64__)
#error "This project only supports linux x86_64"
#endif

#include "lib.h"

long write(int fd, const char* msg, unsigned long msg_len) {
	long ret;
	asm volatile ("syscall" : "=a"(ret) : "a"(SYS_write), "D"(fd), "S"(msg), "d"(msg_len) : "rcx", "r11", "memory");

	if (ret < 0) {
		const char *err = "[FATAL] Write Error\n";
		asm volatile ("syscall" : : "a"(SYS_write), "D"(STDERR_FILENO), "S"(err), "d"(20) : "rcx", "r11", "memory");
		asm volatile ("syscall" : : "a"(SYS_exit), "D"(1) : "rcx", "r11", "memory");
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
		s1++; s2++;
	}
	return *(unsigned char*)s1 - *(unsigned char*)s2;
}

block_header_t *free_list = NULL;

void *malloc(unsigned long size) {
	if (size == 0) return NULL;

	size = ALIGN(size);
	block_header_t *current = free_list;
	block_header_t *last = NULL;

	while (current) {
		if (current->free && current->size >= size) {
            // Tenta dividir o bloco se sobrar espaço para um novo header + payload mínimo
			if (current->size >= size + HEADER_SIZE + ALIGNMENT) {
				block_header_t *new_block = (block_header_t *)((char *)(current + 1) + size);
				new_block->size = current->size - size - HEADER_SIZE;
				new_block->free = 1;
				new_block->next = current->next;

				current->size = size;
				current->next = new_block;
			}
			current->free = 0;
			return (void *)(current + 1);
		}
		last = current;
		current = current->next;
	}

	unsigned long total_to_request = ALIGN(size + HEADER_SIZE);
	unsigned long alloc_size = (total_to_request + 4095) & ~4095;

	block_header_t *block = (block_header_t *)guicall(SYS_mmap, NULL, alloc_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

	if (block == MAP_FAILED) return NULL;

	block->size = alloc_size - HEADER_SIZE;
	block->free = 0;
	block->next = NULL;

	if (!free_list) {
		free_list = block;
	} else {
		last->next = block;
	}

	return (void *)(block + 1);
}

void free(void *ptr)
{
	if (!ptr) return;

	block_header_t *header = (block_header_t *)ptr - 1;
	header->free = 1;

    // Coalescência básica: Juntar blocos livres vizinhos para evitar fragmentação
	block_header_t *cur = free_list;
	while (cur && cur->next) {
		if (cur->free && cur->next->free) {
			// Verifica se são fisicamente contíguos
			if ((char *)(cur + 1) + cur->size == (char *)cur->next) {
				cur->size += HEADER_SIZE + cur->next->size;
				cur->next = cur->next->next;
				continue; // Tenta juntar o próximo novamente
			}
		}
		cur = cur->next;
	}
}

void *realloc(void *ptr, unsigned long size)
{
	if (!ptr) return malloc(size);
	if (size == 0) { free(ptr); return NULL; }

	size = ALIGN(size);
	block_header_t *header = (block_header_t *)ptr - 1;

	if (header->size >= size) return ptr;

	void *new_ptr = malloc(size);
	if (!new_ptr) return NULL;

	unsigned char *src = (unsigned char *)ptr;
	unsigned char *dst = (unsigned char *)new_ptr;
	for (unsigned long i = 0; i < header->size; i++) dst[i] = src[i];

	free(ptr);
	return new_ptr;
}
