
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
*   This is a private header for stdio.h header.
*
*
*END************************************************************************/
#ifndef __dopr_prv_h__
#define __dopr_prv_h__

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * Type definitions also used for sizing by doprint
 * They are the maximum string size that a 32/64 bit number
 * can be displayed as.
 */
#define _MQX_IO_DIVISION_ADJUST_64 1000000000000000000LL
#define PRINT_OCTAL_64   (22L)
#define PRINT_DECIMAL_64 (20L)
#define PRINT_HEX_64     (16L)

#define _MQX_IO_DIVISION_ADJUST 1000000000L
#define PRINT_OCTAL   (11L)
#define PRINT_DECIMAL (10L)
#define PRINT_HEX     (8L)

#define PRINT_ADDRESS (8L)


typedef int (*PUTCHAR_FUNC)(int, FILE *);

int _io_doprint(FILE *farg, PUTCHAR_FUNC func_ptr, int max_count, char  *fmt_ptr, va_list args_ptr);
int _io_sputc(int c, FILE * input_string);

#endif
