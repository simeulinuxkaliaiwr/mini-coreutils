/*
 * @file lib.c
 * @brief Implementation of basic string manipulation functions.
 *
 * This file contains implementations for several common string utilities, these
 * functions are functional equivalents of standart C library functions found in
 * <string.h>.
 *
 * @author simeulinuxkaliaiwr
 * @date December 2025
 * @license MIT
 */

#include "lib.h"
#include <unistd.h>

/**
 * @brief Calculates the length of a null-terminated string.
 *
 * Functionally equivalent to the standard C library function `strlen`.
 * It counts the number of characters in the string until it encounters
 * the null terminator ('\0').
 */

size_t guilen(const char *str) {
  size_t counter = 0;
  while (str[counter] != '\0') {
    ++counter;
  }
  return counter;
}

/**
 * @brief Lexicographically compares two null-terminated strings.
 * Functionally equivalent to the standard C library function `strcmp`.
 */

int guicmp(const char *str1, const char *str2) {
  while (*str1 != '\0') {
    if (*str1 != *str2) {
      break;
    }
    ++str1;
    ++str2;
  }
  return (int)((unsigned char)*str1 - (unsigned char)*str2);
}

/**
 * @brief Copies the source string to the destination buffer.
 *
 * Functionally equivalent to the standard C library function `strcpy`.
 */

void guicpy(char *dest, const char *src) {
  while (*src != '\0') {
    *dest = *src;
    ++dest;
    ++src;
  }
  *dest = '\0';
}

int guincpy(char *dest, const char *src, size_t n) {
  size_t i = 0;
  while (i < n - 1 && src[i] != '\0') {
    dest[i] = src[i];
    ++i;
  }
  if (n > 0) {
    dest[i] = '\0';
  }
  return i;
}

/**
 * @brief Appends the source string to the end of the destination string.
 *
 * Functionally equivalent to the standard C library function `strcat`.
 * The first character of the source string overwrites the null terminator
 * of the destination string.
 *
 */
void guicat(char *dest, const char *src) {
  while (*dest != '\0') {
    ++dest;
  }
  guicpy(dest, src);
}
