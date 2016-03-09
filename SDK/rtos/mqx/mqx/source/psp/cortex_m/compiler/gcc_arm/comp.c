
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
* See license agreement file for full license terms including other restrictions.
*****************************************************************************
*
* Comments:
*
*   This file contains runtime support for the ARM GCC.
*
*
*END************************************************************************/

#include <stdio.h>
#include "mqx.h"

void *malloc(size_t bytes);
void *calloc(size_t n, size_t z);
void free(void *);
void exit(int);
/*! \cond DOXYGEN_PRIVATE */
void _exit(int);
/*! \endcond */

/*! \cond DOXYGEN_PRIVATE */
void __libc_init_array(void);
/*! \endcond */

/*! \cond DOXYGEN_PRIVATE */
void __libc_fini_array(void);
/*! \endcond */

/*! \cond DOXYGEN_PRIVATE */
int atexit(void (*func)(void));
/*! \endcond */

int  main(void);

void *__dso_handle = NULL;

/*!
 * \cond DOXYGEN_PRIVATE
 * \brief Dummy function which avoid GCC to register destructors of global objects.
 *  By default GCC use '__cxa_atexit' function which register destructors to
 *  LIFO. When LIFO is full, it performs malloc, but at this point there is
 *  no heap, so malloc fails.
 *
 * \return int
 */
int __cxa_atexit (void (*fn) (void *), void *arg, void *d)
{
    return 0;
}
/*! \endcond */

/*!
 * \cond DOXYGEN_PRIVATE
 * \brief Perform custom action before main
 */
void _init(void)
{
    ;
}
/*! \endcond */

/*!
 * \cond DOXYGEN_PRIVATE
 * \brief Perform custom action before exit
 */
void _fini(void)
{
    ;
}
/*! \endcond */

#if MQXCFG_ALLOCATOR != MQX_ALLOCATOR_NONE
/*!
 * \brief Override C/C++ runtime heap allocation function
 *
 * \param bytes
 *
 * \return pointer
 */
void *malloc(size_t bytes)
{
    return _mem_alloc_system(bytes);
}

/*!
 * \brief Override C/C++ reentrant runtime heap allocation function
 *
 * \param reent
 * \param nbytes
 *
 * \return pointer
 */
///void *_malloc_r(struct _reent *reent, size_t nbytes)
///{
///    return _mem_alloc_system(nbytes);
///}

/*!
 * \brief Override C/C++ runtime heap deallocation
 *
 * \param n
 * \param z
 *
 * \return pointer
 */
void *calloc(size_t n, size_t z)
{
    return _mem_alloc_system_zero(n*z);
}

/*!
 * \brief Override C/C++ runtime heap deallocation function
 *
 * \param p
 */
void free(void *p)
{
    _mem_free(p);
}
#endif /* MQXCFG_ALLOCATOR */

typedef struct {
    uint32_t *  TARGET;
    uint32_t    BYTESIZE;
} STARTUP_ZEROTABLE_STRUCT, * STARTUP_ZEROTABLE_STRUCT_PTR;

typedef struct {
    uint32_t *  SOURCE;
    uint32_t *  TARGET;
    uint32_t    BYTESIZE;
} STARTUP_COPYTABLE_STRUCT, * STARTUP_COPYTABLE_STRUCT_PTR;


extern STARTUP_ZEROTABLE_STRUCT __START_BSS[];
extern STARTUP_ZEROTABLE_STRUCT __END_BSS[];
extern STARTUP_COPYTABLE_STRUCT __DATA_ROM[];
extern STARTUP_COPYTABLE_STRUCT __DATA_END[];


/*!
 * \cond DOXYGEN_PRIVATE
 * \brief _start is dummy function for weak function Reset_Handler in startup_<proc>.S to avoid linker error
 */
void _start(void)
{
    return;
}
/*! \endcond */

/*!
 * \cond DOXYGEN_PRIVATE
 * \brief Perform necessary toolchain startup routines before main()
 */
void toolchain_startup(void)
{
// register destructor calls of static objects
    atexit(__libc_fini_array);
// run constructor calls of static objects
    __libc_init_array();
// run main, if return go to exit
    exit(main());
}
/*! \endcond */

/*!
 * \cond DOXYGEN_PRIVATE
 * \brief Required implementation
 *
 * \param status
 */
void _exit(int status)
{
// disable all interrupts, run infinite loop
    __asm("cpsid i");
    while(1);
}
/*! \endcond */

/*!
 * \cond DOXYGEN_PRIVATE
 * \brief Required implementation, we don't use this legacy feature for more information
 *  see http//www.unix.com/man-page/FreeBSD/2/sbrk/
 *
 * \param increment
 */
void *_sbrk(intptr_t increment)
{
    _exit(-1);
    return 0;
}
/*! \endcond */

