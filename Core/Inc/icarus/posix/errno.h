/**
 * @file    errno.h
 * @brief   ICARUS OS POSIX Error Numbers
 * @version 0.1.0
 *
 * @details POSIX.1-2017 compatible error number definitions.
 *
 * @see     IEEE Std 1003.1-2017 (POSIX.1)
 * @see     https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/errno.h.html
 *
 * @author  Souham Biswas
 * @date    2026
 *
 * @copyright Copyright (c) 2026 ICARUS Project
 */

#ifndef ICARUS_POSIX_ERRNO_H
#define ICARUS_POSIX_ERRNO_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * ERROR NUMBER VARIABLE
 * ========================================================================= */

/**
 * @brief Thread-local error number
 * @note  In single-core ICARUS, this is a simple global variable
 */
extern int errno;

/* ============================================================================
 * POSIX ERROR CODES
 * ========================================================================= */

#define EOK             0       /**< No error */
#define EPERM           1       /**< Operation not permitted */
#define ENOENT          2       /**< No such file or directory */
#define ESRCH           3       /**< No such process */
#define EINTR           4       /**< Interrupted system call */
#define EIO             5       /**< I/O error */
#define ENXIO           6       /**< No such device or address */
#define E2BIG           7       /**< Argument list too long */
#define ENOEXEC         8       /**< Exec format error */
#define EBADF           9       /**< Bad file descriptor */
#define ECHILD          10      /**< No child processes */
#define EAGAIN          11      /**< Try again / Resource temporarily unavailable */
#define ENOMEM          12      /**< Out of memory */
#define EACCES          13      /**< Permission denied */
#define EFAULT          14      /**< Bad address */
#define ENOTBLK         15      /**< Block device required */
#define EBUSY           16      /**< Device or resource busy */
#define EEXIST          17      /**< File exists */
#define EXDEV           18      /**< Cross-device link */
#define ENODEV          19      /**< No such device */
#define ENOTDIR         20      /**< Not a directory */
#define EISDIR          21      /**< Is a directory */
#define EINVAL          22      /**< Invalid argument */
#define ENFILE          23      /**< File table overflow */
#define EMFILE          24      /**< Too many open files */
#define ENOTTY          25      /**< Not a typewriter */
#define ETXTBSY         26      /**< Text file busy */
#define EFBIG           27      /**< File too large */
#define ENOSPC          28      /**< No space left on device */
#define ESPIPE          29      /**< Illegal seek */
#define EROFS           30      /**< Read-only file system */
#define EMLINK          31      /**< Too many links */
#define EPIPE           32      /**< Broken pipe */
#define EDOM            33      /**< Math argument out of domain */
#define ERANGE          34      /**< Math result not representable */

/* Extended error codes */
#define EDEADLK         35      /**< Resource deadlock would occur */
#define ENAMETOOLONG    36      /**< File name too long */
#define ENOLCK          37      /**< No record locks available */
#define ENOSYS          38      /**< Function not implemented */
#define ENOTEMPTY       39      /**< Directory not empty */
#define ELOOP           40      /**< Too many symbolic links */
#define EWOULDBLOCK     EAGAIN  /**< Operation would block */
#define ENOMSG          42      /**< No message of desired type */
#define EIDRM           43      /**< Identifier removed */

/* Synchronization errors */
#define ENOTSUP         95      /**< Operation not supported */
#define ETIMEDOUT       110     /**< Connection timed out */
#define EOVERFLOW       75      /**< Value too large for data type */
#define EMSGSIZE        90      /**< Message too long */

#ifdef __cplusplus
}
#endif

#endif /* ICARUS_POSIX_ERRNO_H */
