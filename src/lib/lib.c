/*
 * @file lib.c
 * @brief low-level implementation of core utilities for a minimal runtime.
 *
 * This file contains implementations for string, memory and conversion
 * utilities, designed to operate without the standart C library (libc). It
 * relies on direct system calls (via sys/syscall.h) where necessary (e.g.,
 * error handling that requires interaction with the kernel/OS state)
 *
 * @author simeulinuxkaliaiwr
 * @date December 2025
 * @license MIT
 */

#define _GNU_SOURCE

#include <sys/syscall.h>
#include <errno.h>
#include <stddef.h>
#include <unistd.h>
#include <asm/unistd.h>
#include "lib.h"

#define LOW_LEVEL_LONG_MAX 9223372036854775807L       // 2^63 - 1
#define LOW_LEVEL_LONG_MIN (-LOW_LEVEL_LONG_MAX - 1L) // -2^63

extern int errno;

static const char *const _error_msgs[] = {
    [0] = "No error information",
    [EPERM] = "Operation not permitted",
    [ENOENT] = "No such file or directory",
    [ESRCH] = "No such process",
    [EINTR] = "Interrupted system call",
    [EIO] = "Input/output error",
    [ENXIO] = "No such device or address",
    [E2BIG] = "Argument list too long",
    [ENOEXEC] = "Exec format error",
    [EBADF] = "Bad file descriptor",
    [ECHILD] = "No child processes",
    [EAGAIN] = "Resource temporarily unavailable",
    [ENOMEM] = "Cannot allocate memory",
    [EACCES] = "Permission denied",
    [EFAULT] = "Bad address",
    [EBUSY] = "Device or resource busy",
    [EEXIST] = "File exists",
    [EXDEV] = "Invalid cross-device link",
    [ENODEV] = "No such device",
    [ENOTDIR] = "Not a directory",
    [EISDIR] = "Is a directory",
};

static const size_t _NUM_ERRORS = sizeof(_error_msgs) / sizeof(_error_msgs[0]);

/**
 * @brief Copies 'n' bytes from the memory area 'src' to 'dest'.
 *
 * Functionally equivalent to 'memcpy'. The copy is done byte by byte.
 * Assumes that the areas do not overlap (default behavior of memcpy).
 */

void *guimemcpy(void *dest, const void *src, size_t n) {
  char *d = (char *)dest;
  const char *s = (const char *)src;
  while (n--) {
    *d++ = *s++;
  }
  return dest;
}

/**
 * @brief Fills the first 'n' bytes of the memory area pointed to by 's'
 *
 * with the constant byte 'c'.
 * Functionally equivalent to 'memset'.
 */

void *guimemset(void *s, int c, size_t n) {
  char *p = (char *)s;
  const unsigned char uc = (unsigned char)c;
  while (n--) {
    *p++ = uc;
  }
  return s;
}

/**
 * @brief Compares the first 'n' bytes of memory areas 's1' and 's2'.
 *
 * Functionally equivalent to 'memcmp'.
 */

int guimemcmp(const void *s1, const void *s2, size_t n) {
  const unsigned char *p1 = (const unsigned char *)s1;
  const unsigned char *p2 = (const unsigned char *)s2;
  for (size_t i = 0; i < n; ++i) {
    if (p1[i] != p2[i]) {
      return (int)(p1[i] - p2[i]);
    }
  }
  return 0;
}

/*
 * ==================
 * =String functions=
 * ==================
 */

/**
 * @brief Calculates the length of a null-terminated string.
 *
 * (equivalent to strlen)
 */

size_t guilen(const char *str) {
  size_t counter = 0;
  while (str[counter] != '\0') {
    ++counter;
  }
  return counter;
}

/**
 * @brief Calculates the lenght of a string, limiting it to a search of 'maxlen'
 * bytes (Equivalent to strnlen)
 */

size_t guinlen(const char *str, size_t maxlen) {
  size_t len = 0;
  while (len < maxlen && str[len] != '\0') {
    ++len;
  }
  return len;
}

int guicmp(const char *s1, const char *s2) {
  while (*s1 && (*s1 == *s2)) {
    ++s1;
    ++s2;
  }
  return (int)*((const unsigned char *)s1) - (int)*((const unsigned char *)s2);
}

int guincmp(const char *s1, const char *s2, size_t n) {
  while (n > 0) {
    if (*s1 != *s2) {
      return (int)*((const unsigned char *)s1) -
             (int)*((const unsigned char *)s2);
    }
    if (*s1 == '\0') {
      return 0;
    }
    ++s1;
    ++s2;
    n--;
  }
  return 0;
}

/**
 * @brief Copies the string of 'src' to 'dest'.
 * (Equivalent to strcpy)
 * @return Return 'dest'.
 */

char *guicpy(char *dest, const char *src) {
  char *orig_dest = dest;
  while ((*dest++ = *src++)) {
    // No need for nothing here.
  }
  return orig_dest;
}

/**
 * @brief Copies a maximum of 'n' bytes from 'src' to 'dest'.
 * (equivalent to strncpy)
 *
 * WARNING: strncpy has a peculiar behavior - if the length of 'src'
 * is less than 'n', the remainder of 'dest' is padded with '\0'.
 * This implementation replicates this behavior.
 *
 * @return Returns 'dest'.
 */

char *guincpy(char *dest, const char *src, size_t n) {
  char *orig_dest = dest;
  size_t i = 0;
  while (i < n && src[i] != '\0') {
    dest[i] = src[i];
    ++i;
  }
  while (i < n) {
    dest[i] = '\0';
    ++i;
  }
  return orig_dest;
}

/**
 * @brief Adds 'src' at the end of 'dest'.
 * (Equivalent to strcat)
 * @return Returns 'dest'.
 */

char *guicat(char *dest, const char *src) {
  char *orig_dest = dest;
  while (*dest != '\0') {
    ++dest;
  }
  guicpy(dest, src);
  return orig_dest;
}

/**
 * @brief Adds at most 'n' bytes of 'src' to the end of 'dest'.
 * (Equivalent to strncat)
 *
 * Ensures that the result ends in zero.
 *
 * @return Returns 'dest'.
 */

char *guincat(char *dest, const char *src, size_t n) {
  char *orig_dest = dest;
  size_t dest_len = guilen(dest);
  size_t i;
  dest += dest_len;
  for (i = 0; i < n && src[i] != '\0'; ++i) {
    dest[i] = src[i];
  }
  dest[i] = '\0';
  return orig_dest;
}

/*
 * ==============================
 * =String to number conversions=
 * ==============================
 */

long guitol(const char *str, char **endptr, int base) {
  long result = 0;
  int sign = 1;
  const char *p = str;

  while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' || *p == '\f' ||
         *p == '\v') {
    p++;
  }

  if (*p == '-') {
    sign = -1;
    p++;
  } else if (*p == '+') {
    p++;
  }

  if (base == 0) {
    if (*p == '0') {
      if (*(p + 1) == 'x' || *(p + 1) == 'X') {
        base = 16;
        p += 2;
      } else {
        base = 8;
      }
    } else {
      base = 10;
    }
  } else if (base == 16) {
    if (*p == '0' && (*(p + 1) == 'x' || *(p + 1) == 'X')) {
      p += 2;
    }
  }

  if (base < 2 || base > 36) {
    if (endptr)
      *endptr = (char *)str;
    return 0;
  }

  long threshold = LOW_LEVEL_LONG_MAX / base;
  long remainder = LOW_LEVEL_LONG_MAX % base;

  int overflow_flag = 0;

  unsigned char digit;
  int digit_val;

  // 4. Conversão
  while (*p != '\0') {
    digit = (unsigned char)*p;

    if (digit >= '0' && digit <= '9') {
      digit_val = digit - '0';
    } else if (digit >= 'a' && digit <= 'z') {
      digit_val = digit - 'a' + 10;
    } else if (digit >= 'A' && digit <= 'Z') {
      digit_val = digit - 'A' + 10;
    } else {
      break;
    }

    if (digit_val >= base) {
      break;
    }

    if (sign == 1) {
      if (result > threshold ||
          (result == threshold && digit_val > remainder)) {
        overflow_flag = 1;
        break;
      }
    } else {
      long negative_threshold = LOW_LEVEL_LONG_MIN / base;
      long negative_remainder = LOW_LEVEL_LONG_MIN % base;

      if (result < negative_threshold ||
          (result == negative_threshold && digit_val > -(negative_remainder))) {
        overflow_flag = 1;
        break;
      }
    }

    if (sign == 1) {
      result = (result * base) + digit_val;
    } else {
      result = (result * base) - digit_val;
    }

    p++;
  }

  if (endptr) {
    *endptr = (char *)p;
  }

  if (overflow_flag) {
    return (sign == 1) ? LOW_LEVEL_LONG_MAX : LOW_LEVEL_LONG_MIN;
  }

  return result;
}

/**
 * @brief converts a string to a 'int'.
 * (Equivalent to atoi, unsing guitol)
 */

int guitoi(const char *str) { return (int)guitol(str, NULL, 10); }

/*
 * ================
 * =Error Handling=
 * ================
 */

/**
 * @brief Converts a error number (errno) into a descritive string.
 * (Equivalent to strerror).
 */

const char *gui_strerror(int errnum) {
  if (errnum > 0 && (size_t)errnum < _NUM_ERRORS && _error_msgs[errnum] != NULL) {
    return _error_msgs[errnum];
  }
  return "Unknown error";
}

/**
 * @brief Prints a error message in the standart error output (stderr), followed
 * by the error string corresponding to the current value of 'errno'.
 *
 * (Equivalent to perror)
 */

void gui_perror(const char *msg) {
  const char *err_str = gui_strerror(errno);
  const char *colon_space = ": ";
  const char *newline = "\n";

  if (msg != NULL && *msg != '\0') {
    syscall(SYS_write, 2, msg, guilen(msg));
    syscall(SYS_write, 2, colon_space, 2);
  }

  // Error string
  syscall(SYS_write, 2, err_str, guilen(err_str));
  // Newline
  syscall(SYS_write, 2, newline, 1);
}
