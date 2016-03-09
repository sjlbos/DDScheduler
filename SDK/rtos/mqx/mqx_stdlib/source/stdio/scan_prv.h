
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
*   This is a private header.
*
*
*END************************************************************************/
#ifndef __scan_prv_h__
#define __scan_prv_h__

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

#define SCAN_ERROR    (-1)
#define SCAN_LLONG    (0)
#define SCAN_WLONG    (1)
#define SCAN_BLONG    (2)
#define SCAN_MLONG    (3)

#ifndef HUGE_VAL
#define HUGE_VAL         (99.e99)///wrong value
#endif

int io_scan_prv(const char *line_ptr, char *format, va_list args_ptr);

#endif
