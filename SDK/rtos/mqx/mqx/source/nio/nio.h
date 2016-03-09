#ifndef __nio_h__
#define __nio_h__
/*HEADER**********************************************************************
 *
 * Copyright 2013 Freescale Semiconductor, Inc.
 *
 * This software is owned or controlled by Freescale Semiconductor.
 * Use of this software is governed by the Freescale MQX RTOS License
 * distributed with this Material.
 * See the MQX_RTOS_LICENSE file distributed for more details.
 *
 * Brief License Summary:
 * This software is provided in source form for you to use free of charge,
 * but it is not open source software. You are allowed to use this software
 * but you cannot redistribute it or derivative works of it in source form.
 * The software may be used only in connection with a product containing
 * a Freescale microprocessor, microcontroller, or digital signal processor.
 * See license agreement file for full license terms including other
 * restrictions.
 *
 *END************************************************************************/

#include <stdint.h>
#include <stdarg.h>
#include "fsl_os_abstraction.h"

/* NIO's errno... TODO: move to standard library */
#if defined ( __IAR_SYSTEMS_ICC__ )
/* MISRA C 2004 rule 20.1 suppress: Error[Pm098]: Reserved words and standard library function names shall not be redefined or undefined */
/* MISRA C 2004 rule 20.2 suppress: Error[Pm150]: The names of standard library macros, objects, and functions shall not be reused */
_Pragma ("diag_suppress = Pm098, Pm150")
#endif
#if defined ( __IAR_SYSTEMS_ICC__ )
_Pragma ("diag_default = Pm098, Pm150")
#endif

/* NIO- specific error codes */
#define NIO_EPERM           1               /* Operation not permitted */
#define NIO_ENOENT          2               /* No such file or directory */
#define NIO_ESRCH           3               /* No such process */
#define NIO_EINTR           4               /* Interrupted system call */
#define NIO_EIO             5               /* Input/output error */
#define NIO_ENXIO           6               /* Device not configured */
#define NIO_E2BIG           7               /* Argument list too long */
#define NIO_ENOEXEC         8               /* Exec format error */
#define NIO_EBADF           9               /* Bad file descriptor */
#define NIO_ECHILD          10              /* No child processes */
#define NIO_EDEADLK         11              /* Resource deadlock avoided */
                                        /* 11 was EAGAIN */
#define NIO_ENOMEM          12              /* Cannot allocate memory */
#define NIO_EACCES          13              /* Permission denied */
#define NIO_EFAULT          14              /* Bad address */

#ifndef _POSIX_SOURCE
#define NIO_ENOTBLK         15              /* Block device required */
#endif

#define NIO_EBUSY           16              /* Device busy */
#define NIO_EEXIST          17              /* File exists */
#define NIO_EXDEV           18              /* Cross-device link */
#define NIO_ENODEV          19              /* Operation not supported by device */
#define NIO_ENOTDIR         20              /* Not a directory */
#define NIO_EISDIR          21              /* Is a directory */
#define NIO_EINVAL          22              /* Invalid argument */
#define NIO_ENFILE          23              /* Too many open files in system */
#define NIO_EMFILE          24              /* Too many open files */
#define NIO_ENOTTY          25              /* Inappropriate ioctl for device */

#ifndef _POSIX_SOURCE
#define NIO_ETXTBSY         26              /* Text file busy */
#endif

#define NIO_EFBIG           27              /* File too large */
#define NIO_ENOSPC          28              /* No space left on device */
#define NIO_ESPIPE          29              /* Illegal seek */
#define NIO_EROFS           30              /* Read-only filesystem */
#define NIO_EMLINK          31              /* Too many links */
#define NIO_EPIPE           32              /* Broken pipe */

#if defined ( __IAR_SYSTEMS_ICC__ )
/* MISRA C 2004 rule 20.1 suppress: Error[Pm098]: Reserved words and standard library function names shall not be redefined or undefined */
/* MISRA C 2004 rule 20.2 suppress: Error[Pm150]: The names of standard library macros, objects, and functions shall not be reused */
_Pragma ("diag_suppress = Pm098,Pm150")
/* math software */
#define NIO_EDOM            33              /* Numerical argument out of domain */
#define NIO_ERANGE          34              /* Result too large */
_Pragma ("diag_default = Pm098, Pm150")
#endif

/* non-blocking and interrupt i/o */
#define NIO_EAGAIN          35              /* Resource temporarily unavailable */

#ifndef _POSIX_SOURCE
#define NIO_EWOULDBLOCK     NIO_EAGAIN          /* Operation would block */
#define NIO_EINPROGRESS     36              /* Operation now in progress */
#define NIO_EALREADY        37              /* Operation already in progress */

/* ipc/network software -- argument errors */
#define NIO_ENOTSOCK        38              /* Socket operation on non-socket */
#define NIO_EDESTADDRREQ    39              /* Destination address required */
#define NIO_EMSGSIZE        40              /* Message too long */
#define NIO_EPROTOTYPE      41              /* Protocol wrong type for socket */
#define NIO_ENOPROTOOPT     42              /* Protocol not available */
#define NIO_EPROTONOSUPPORT 43              /* Protocol not supported */
#define NIO_ESOCKTNOSUPPORT 44              /* Socket type not supported */
#define NIO_EOPNOTSUPP      45              /* Operation not supported */
#define NIO_ENOTSUP         NIO_EOPNOTSUPP      /* Operation not supported */
#define NIO_EPFNOSUPPORT    46              /* Protocol family not supported */
#define NIO_EAFNOSUPPORT    47              /* Address family not supported by protocol family */
#define NIO_EADDRINUSE      48              /* Address already in use */
#define NIO_EADDRNOTAVAIL   49              /* Can't assign requested address */

/* ipc/network software -- operational errors */
#define NIO_ENETDOWN        50              /* Network is down */
#define NIO_ENETUNREACH     51              /* Network is unreachable */
#define NIO_ENETRESET       52              /* Network dropped connection on reset */
#define NIO_ECONNABORTED    53              /* Software caused connection abort */
#define NIO_ECONNRESET      54              /* Connection reset by peer */
#define NIO_ENOBUFS         55              /* No buffer space available */
#define NIO_EISCONN         56              /* Socket is already connected */
#define NIO_ENOTCONN        57              /* Socket is not connected */
#define NIO_ESHUTDOWN       58              /* Can't send after socket shutdown */
#define NIO_ETOOMANYREFS    59              /* Too many references: can't splice */
#define NIO_ETIMEDOUT       60              /* Operation timed out */
#define NIO_ECONNREFUSED    61              /* Connection refused */

#define NIO_ELOOP           62              /* Too many levels of symbolic links */

#define NIO_EOVERFLOW       75              /* Value too large for defined data type */
#endif /* _POSIX_SOURCE */


/** io defines
 */
//The maximum driver name length including zero-ending character
#define NIO_DEV_NAME_LEN        20

/** open function type for low level driver
 */
typedef int64_t _nio_off_t;
typedef int (*NIO_OPEN_FPTR)(void *dev_context, const char *dev_name, int flags, void **fp_context, int *error);
typedef int (*NIO_READ_FPTR)(void *dev_context, void *fp_context, void *buf, size_t nbytes, int *error);
typedef int (*NIO_WRITE_FPTR)(void *dev_context, void *fp_context, const void *buf, size_t nbytes, int *error);
typedef _nio_off_t (*NIO_LSEEK_FPTR)(void *dev_context, void *fp_context, _nio_off_t offset, int whence, int *error);
typedef int (*NIO_IOCTL_FPTR)(void *dev_context, void *fp_context, int *error, unsigned long int request, va_list ap);
typedef int (*NIO_CLOSE_FPTR)(void *dev_context, void *fp_context, int *error);
typedef int (*NIO_INIT_FPTR)(void *init_data, void **dev_context, int *error);
typedef int (*NIO_DEINIT_FPTR)(void *dev_context, int *error);

/** io device functions callbacks
 */
typedef struct nio_dev_fn_struct {
    NIO_OPEN_FPTR OPEN;            ///< io driver open fn
    NIO_READ_FPTR READ;            ///< io driver read fn
    NIO_WRITE_FPTR WRITE;          ///< io driver write fn
    NIO_LSEEK_FPTR LSEEK;          ///< io driver lseek fn
    NIO_IOCTL_FPTR IOCTL;          ///< io driver ioctl fn
    NIO_CLOSE_FPTR CLOSE;          ///< io driver close fn
    NIO_INIT_FPTR INIT;            ///< io driver initialization fn (call from install)
    NIO_DEINIT_FPTR DEINIT;        ///< io driver deinitialization fn (call from uninstall)
} NIO_DEV_FN_STRUCT;

/** io device
 */
typedef struct nio_dev_struct {
    const NIO_DEV_FN_STRUCT *FN;     ///< pointer to io driver fn list
    void *CONTEXT;              ///< device context - deviced specific allocated data
    uint32_t USAGE;             ///< usage counter, counting actual  running io driver fn calls (read, write, ioctl)
    semaphore_t LOCK;          ///< device lock

    struct nio_file *FP;           ///< opened fp list for device
} NIO_DEV_STRUCT;

/** Initialize io subsystem.
 * \return successful completion return 0
 */
int _nio_init(int max_fd, int *error);

/** Install io device.
 * Function install io device to the system. If device is installed in system, return this installed device.
 * \param name < name of installing device
 * \param fn < device low level functions list
 * \param init_data < io device initialization data
 * \return successful completion return pointer to allocated io device structure
 */
NIO_DEV_STRUCT* _nio_dev_install(const char *name, const NIO_DEV_FN_STRUCT *fn, void *init_data, int *error);

/** Uninstall io device
 * \param name < name of uninstalling device
 * \return successful completion return 0
 */
int _nio_dev_uninstall(const char *name, int *error);

/** Force uninstall io device
 * \param name < name of uninstalling device
 * \return successful completion return 0
 */
int _nio_dev_uninstall_force(const char *name, int *error);

/** Open file or device
 * \param name <
 * \param flags <
 * \param mode <
 * \return return the new file descriptor, or -1 if an error occurred (errno)
 */
int _nio_open(const char *name, int flags, int *error);

/** Read from a file descriptor
 *  Read up to count bytes from file descriptor fd into the buffer starting at buf.
 * \param fd < file descriptor
 * \param buf < buffer start address
 * \param nbytes < count bytes for read
 * \return On success, the number of bytes read is returned, on error,
 * -1 is returned, and errno is set.
 */
ssize_t _nio_read(int fd, void *buf, size_t nbytes, int *error);

/** Write to a file descriptor
 *  Writes up to count bytes from the buffer pointed buf to the file referred to by the file descriptor fd.
 * \param fd < file descriptor
 * \param buf < buffer start address
 * \param nbytes < count bytes for write
 * \return On success, the number of bytes written is returned, on error,
 * -1 is returned, and errno is set to indicate the error
 */
ssize_t _nio_write(int fd, const void *buf, size_t nbytes, int *error);

/** Move the read/write file offset
 * The function shall set the file offset for the open file description associated with the file descriptor fildes.
 * \param fd < file descriptor
 * \param offset < offset value
 * \param whence < base of offset calculation (SEEK_SET, SEEK_CUR, SEEK_END)
 * \return Upon successful completion, the resulting offset, as measured in bytes from the beginning of the
 * file, shall be returned. Otherwise, (_nio_off_t)-1 shall be returned, errno shall be set to indicate the
 * error, and the file offset shall remain unchanged.
 * -1 is returned, and errno is set to indicate the error
 */
_nio_off_t _nio_lseek(int fd, _nio_off_t offset, int whence, int *error);

/** IOCTL - control device
 * \param fd < file descriptor
 * \param request < request code
 * \return On success, return >=0, on error -1 is returned, and errno is set to indicate the error
 */
int _nio_ioctl(int fd, int *error, unsigned long int request, ...);
int _nio_vioctl(int fd, int *error, unsigned long int request, va_list ap);


/** Close a file descriptor
 * \param fd < file descriptor
 * \return successful completion return 0. Otherwise -1 is returned and errno is set to indicate the error
 */
int _nio_close(int fd, int *error);

#endif // __nio_h__
