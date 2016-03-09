/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
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
*****************************************************************************
*
* Comments:
*
*   MQX configuration set: for small-RAM devices
*
*
*END************************************************************************/

#ifndef __small_ram_config_h__
#define __small_ram_config_h__

#ifndef MQXCFG_INIT_TASK
#define MQXCFG_INIT_TASK 1
#endif

#ifndef MQXCFG_EXCEPTION_HANDLING
#define MQXCFG_EXCEPTION_HANDLING 0
#endif

#ifndef MQXCFG_MAX_FD
#define MQXCFG_MAX_FD 20
#endif

/*
** When MQX_CUSTOM_INIT is defined as 1,
** user can start mqx in main function.
*/
#ifndef MQX_CUSTOM_MAIN
#define MQX_CUSTOM_MAIN 0
#endif

#ifndef BSP_FIRST_INTERRUPT_VECTOR_USED
#define BSP_FIRST_INTERRUPT_VECTOR_USED          (0)
#endif

#ifndef BSP_LAST_INTERRUPT_VECTOR_USED
#define BSP_LAST_INTERRUPT_VECTOR_USED           (PSP_MAXIMUM_INTERRUPT_VECTORS)
#endif

#ifndef BSPCFG_ENABLE_IO_SUBSYSTEM
#define BSPCFG_ENABLE_IO_SUBSYSTEM               (1)
#endif

#ifndef MQXCFG_ALLOCATOR
#define MQXCFG_ALLOCATOR                         MQX_ALLOCATOR_LWMEM
#endif


#ifndef MQXCFG_PREALLOCATED_SYSTEM_STACKS
#define MQXCFG_PREALLOCATED_SYSTEM_STACKS        0
#endif

#ifndef MQXCFG_PREALLOCATED_READYQ
#define MQXCFG_PREALLOCATED_READYQ               0
#endif

#ifndef MQXCFG_STATIC_MUTEX
#define MQXCFG_STATIC_MUTEX                      0
#endif

#ifndef MQXCFG_STATIC_LWLOG
#define MQXCFG_STATIC_LWLOG                      0
#endif

#ifndef MQXCFG_STATIC_KLOG
#define MQXCFG_STATIC_KLOG                       0
#endif


#ifndef MQXCFG_PRINT_MEM_DUMP
#define MQXCFG_PRINT_MEM_DUMP                    0
#endif

#ifndef MQXCFG_STATIC_FP_CONTEXT
#define MQXCFG_STATIC_FP_CONTEXT                 0
#endif

/*
* Default number of task priorities
*/
#ifndef MQXCFG_LOWEST_TASK_PRIORITY
#define MQXCFG_LOWEST_TASK_PRIORITY              (16)
#endif

#ifndef MQXCFG_SPARSE_ISR_TABLE
#define MQXCFG_SPARSE_ISR_TABLE                  1
#endif

/* Disable heavy weight components */

#ifndef MQX_USE_IPC
#define MQX_USE_IPC                         0
#endif

#ifndef MQX_USE_LOGS
#define MQX_USE_LOGS                        0
#endif

#ifndef MQX_USE_SEMAPHORES
#define MQX_USE_SEMAPHORES                  0
#endif

#ifndef MQX_USE_SW_WATCHDOGS
#define MQX_USE_SW_WATCHDOGS                0
#endif

#ifndef MQX_USE_TIMER
#define MQX_USE_TIMER                       0
#endif

/* Other configuration */

#ifndef MQX_DEFAULT_TIME_SLICE_IN_TICKS
#define MQX_DEFAULT_TIME_SLICE_IN_TICKS     1
#endif

#ifndef MQX_LWLOG_TIME_STAMP_IN_TICKS
#define MQX_LWLOG_TIME_STAMP_IN_TICKS       1
#endif

#ifndef MQX_TIMER_USES_TICKS_ONLY
#define MQX_TIMER_USES_TICKS_ONLY           1
#endif

#ifndef MQX_EXTRA_TASK_STACK_ENABLE
#define MQX_EXTRA_TASK_STACK_ENABLE         0
#endif

#ifndef MQX_IS_MULTI_PROCESSOR
#define MQX_IS_MULTI_PROCESSOR              0
#endif

#ifndef MQX_MUTEX_HAS_POLLING
#define MQX_MUTEX_HAS_POLLING               0
#endif

#ifndef MQX_USE_INLINE_MACROS
#define MQX_USE_INLINE_MACROS               0
#endif

/*
** When MQX_ALLOCATOR_GARBAGE_COLLECTING is defined as 1,
** then mqx will do the garbage collectin on task destroy.
*/
#ifndef MQX_ALLOCATOR_GARBAGE_COLLECTING
#define MQX_ALLOCATOR_GARBAGE_COLLECTING    1
#endif

/*
** When MQX_ALLOCATOR_ALLOW_IN_ISR is defined as 1,
** then the allocations are allowed in ISR context
*/
#ifndef MQX_ALLOCATOR_ALLOW_IN_ISR
#define MQX_ALLOCATOR_ALLOW_IN_ISR 1
#endif

#ifndef MQX_USE_IDLE_TASK
#define MQX_USE_IDLE_TASK                   0
#endif

#ifndef MQX_HAS_TIME_SLICE
#define MQX_HAS_TIME_SLICE                  0
#endif

#ifndef MQX_KERNEL_LOGGING
#define MQX_KERNEL_LOGGING                  0
#endif

#ifndef MQX_EXIT_ENABLED
#define MQX_EXIT_ENABLED                    0
#endif

/*
** MFS settings
*/

#ifndef MFSCFG_MINIMUM_FOOTPRINT
#define MFSCFG_MINIMUM_FOOTPRINT            1
#endif


/*
** NIO serial settings
*/

#ifndef NIO_SERIAL_BUFF_SIZE
#define NIO_SERIAL_BUFF_SIZE                64
#endif

#endif /* __small_ram_config_h__ */
