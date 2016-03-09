
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
* See license agreement file for full license terms including other restrictions.
*****************************************************************************
*
* Comments:
*
*
*
*END************************************************************************/

#ifndef __mqx_cpudef_h__
#define __mqx_cpudef_h__

/* Include FSK device registers from Platform SDK */
#include "fsl_device_registers.h"

/* generating macros */
#define PSP_MQX_CPU_IS_PPC              (0)
#define PSP_MQX_CPU_IS_COLDFIRE         (0)
#define PSP_MQX_CPU_IS_ARM_CORTEX_A5    ((defined __CORTEX_A) && (__CORTEX_A == 0x05))

#define PSP_MQX_CPU_IS_ARM_CORTEX_M4    ((defined __CORTEX_M) && (__CORTEX_M == 0x04))
#define PSP_MQX_CPU_IS_ARM_CORTEX_M0P   ((defined __CORTEX_M) && (__CORTEX_M == 0x00))


#define PSP_MQX_CPU_IS_ARM              ( PSP_MQX_CPU_IS_ARM_CORTEX_M0P || \
                                          PSP_MQX_CPU_IS_ARM_CORTEX_M4  || \
                                          PSP_MQX_CPU_IS_ARM_CORTEX_A5)

#define PSP_MQX_CPU_IS_KINETIS          (defined __CORTEX_M)

#endif /* __mqx_cpudef_h__ */
