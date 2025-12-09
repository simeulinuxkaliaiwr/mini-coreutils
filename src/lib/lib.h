#ifndef LIB_H
#define LIB_H
#include <unistd.h>

size_t guilen(const char *str);
int guicmp(const char *str1, const char *str2);
void guicpy(char *dest, const char *src);
int guincpy(char *dest, const char *src, size_t n);
void guicat(char *dest, const char *src);
#endif
