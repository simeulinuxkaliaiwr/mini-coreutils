#include "lib.h"
#include <unistd.h>

size_t guilen(const char *str) {
  size_t counter = 0;
  while (str[counter] != '\0') {
    ++counter;
  }
  return counter;
}

int guicmp(const char *str1, const char *str2) {
  while (*str1 != '\0') {
    if (*str1 != *str2) {
      break;
    }
    ++str1;
    ++str2;
  }
  return (int)*str1 - (int)*str2;
}

void guicpy(char *dest, const char *src) {
  while (*src != '\0') {
    *dest = *src;
    ++dest;
    ++src;
  }
  *dest = '\0';
}

void guicat(char *dest, const char *src) {
  while (*dest != '\0') {
    ++dest;
  }
  guicpy(dest, src);
}
