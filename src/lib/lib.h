#ifndef LIB_H
#define LIB_H

#include <stddef.h>
#include <sys/types.h>
#include <unistd.h>

size_t guilen(const char *str);
size_t guinlen(const char *str, size_t maxlen);
int guicmp(const char *s1, const char *s2);
int guincmp(const char *s1, const char *s2, size_t n);
char *guicpy(char *dest, const char *src);
char *guincpy(char *dest, const char *src, size_t n);
char *guicat(char *dest, const char *src);
char *guincat(char *dest, const char *src, size_t n);

void *guimemcpy(void *dest, const void *src, size_t n);
void *guimemset(void *s, int c, size_t n);
int guimemcmp(const void *s1, const void *s2, size_t n);

long guitol(const char *str, char **endptr, int base);
int guitoi(const char *str);

void gui_perror(const char *msg);
const char *gui_strerror(int errnum);

#endif
