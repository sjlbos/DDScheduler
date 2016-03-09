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
#include "mqx.h"
#include "nio.h"
#include "unistd.h"

/*! \cond DOXYGEN_PRIVATE */
int _WRITE(int fd, const void *buf, size_t nbytes)
{
    return (int)write(fd, buf, nbytes);
}
/*! \endcond */

/*! \cond DOXYGEN_PRIVATE */
int _READ(int fd, void *buf, size_t nbytes)
{
    return (int)read(fd, buf, nbytes);
}
/*! \endcond */

/*! \cond DOXYGEN_PRIVATE */
int _OPEN(const char *name, int flags, int mode)
{
    return open(name, flags, mode);
}
/*! \endcond */

/*! \cond DOXYGEN_PRIVATE */
int _CLOSE(int fd)
{
    return close(fd);
}
/*! \endcond */

/*! \cond DOXYGEN_PRIVATE */
long _LSEEK(int fd, long offset, int whence)
{
    return lseek(fd, offset, whence);
}
/*! \endcond */

/*! \cond DOXYGEN_PRIVATE */
void _STD_LOCK()
{
    _int_disable();
}
/*! \endcond */

/*! \cond DOXYGEN_PRIVATE */
void _STD_UNLOCK()
{
    _int_enable();
}
/*! \endcond */

