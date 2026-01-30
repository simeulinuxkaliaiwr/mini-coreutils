#ifndef SYS_ERRNO_H
#define SYS_ERRNO_H

#define EPERM 1 /* Operation not permitted */
#define ENOENT 2      /* No such file or directory */
#define ESRCH 3      /* No such process */
#define EINTR 4      /* Interrupted system call */
#define EIO 5      /* I/O error */
#define EBADF 9      /* Bad file number */
#define ENOMEM 12      /* Out of memory */
#define EACCES 13      /* Permission denied */
#define EEXIST 17      /* File exists */
#define EINVAL 22      /* Invalid argument */
#define ENOSPC 28      /* No space left on device */
#define ENOTDIR 20
#define EISDIR 21

static inline int extract_error(long syscall_ret)
{
	if (syscall_ret < 0 && syscall_ret > -4096) {
		return (int)(-syscall_ret);
	}
	return 0;
}

#endif
