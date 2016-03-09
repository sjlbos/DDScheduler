
/*HEADER**********************************************************************
*
* Copyright 2008-2012 Freescale Semiconductor, Inc.
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
*   This file contains the structure definitions and constants for an
*   application which will be using MQX.
*   All compiler provided header files must be included before mqx.h.
*
*
*END************************************************************************/

#ifndef __task_h__
#define __task_h__
#include <stdint.h>

typedef struct task_init_struct
{
//    _mqx_uint           index;
   void (*exec)(uint32_t);// TODO: void* instead uint32_t !!
   _mem_size            stacksize;
   void *               stackaddr;
   _mqx_uint            priority;
   char *               name;
   _mqx_uint            attributes;
   void *               exec_param;
   _mqx_uint            time_slice;
} taskinit_t;

uint32_t create_task(const taskinit_t * taskinit_p);

#endif
