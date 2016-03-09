#ifndef __nio_posix_h__
#define __nio_posix_h__
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
/* include toolchain's errno defintion */
#include <errno.h>
#include <psp_comp.h>

#ifdef __cplusplus
extern "C" {
#endif

/** POSIX API definition
 */
int open(const char *name, int flags, ...);
int close(int fd);
ssize_t read(int fd, void *buf, size_t nbytes);
ssize_t write(int fd, const void *buf, size_t nbytes);
int ioctl(int fd, unsigned long int request, ...);
off_t lseek(int fd, off_t offset, int whence);

#ifdef __cplusplus
}
#endif

#endif // __nio_posix_h__
