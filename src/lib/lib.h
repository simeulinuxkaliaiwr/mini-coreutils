#ifndef LIB_H
#define LIB_H
#include <unistd.h>

size_t guilen(const char *str);
int guicmp(const char *str1, const char *str2);
void guicpy(char *dest, const char *src);
void guicat(char *dest, const char *src);
#endif
