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
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <assert.h>
#include "nio.h"
#include "nio_prv.h"
#include "ioctl.h"
#include "fsl_os_abstraction.h"
#if defined(FSL_RTOS_MQX)
#include "mqx_inc.h"
#endif
#include <errno.h>

#if defined ( __IAR_SYSTEMS_ICC__ )
/* MISRA C 2004 rule 20.5 suppress: Error[Pm101]: The error indicator errno shall not be used */
/* MISRA C 2004 rule 14.2 suppress: Error[Pm049]: All non-null statements shall have a side-effect */
/* Pm049 occurs only on va_end() provided by stdarg.h header */
_Pragma ("diag_suppress= Pm101,Pm049")
#endif

int open(const char *name, int flags, ...) 
{
    int error;
    int rv;
    int flgs;

    /* translate flags to flgs, depending on toolchain used */
    flgs = flags;
    if (0 > (rv = _nio_open(name, flgs, &error)))
    {
        /* translate error to errno */
        errno = error;
    }

    return rv;
}

int close(int fd)
{
    int error;
    int rv;

    if (0 > (rv = _nio_close(fd, &error)))
    {
        /* translate error to errno */
        errno = error;
    }

    return rv;
}


ssize_t read(int fd, void *buf, size_t nbytes)
{
    int error;
    ssize_t rv;

    if (0 > (rv = _nio_read(fd, buf, nbytes, &error)))
    {
        /* translate error to errno */
        errno = error;
    }

    return rv;
}

ssize_t write(int fd, const void *buf, size_t nbytes)
{
    int error;
    ssize_t rv;

    if (0 > (rv = _nio_write(fd, buf, nbytes, &error)))
    {
        /* translate error to errno */
        errno = error;
    }

    return rv;
}

int ioctl(int fd, unsigned long int request, ...)
{
    int error;
    ssize_t rv;
    va_list ap;

    va_start(ap, request);

    if (0 > (rv = _nio_vioctl(fd, &error, request, ap)))
    {
        /* translate error to errno */
        errno = error;
    }

    va_end(ap);

    return rv;
}

off_t lseek(int fd, off_t offset, int whence)
{
    int error;
    ssize_t rv;

    if (0 > (rv = _nio_lseek(fd, offset, whence, &error)))
    {
         errno = error;
    }
    
    return rv;
}

