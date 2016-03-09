
/*HEADER**********************************************************************
*
* Copyright 2010-2013 Freescale Semiconductor, Inc.
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

*
*END************************************************************************/

#include "asm_mac.h"
#include "mqx_cnfg.h"
#include "types.inc"
#include "psp_prv.inc"

    ASM_COMP_SPECIFIC_DIRECTIVES
    ASM_CODE_SECTION(.text)
    SET_FUNCTION_ALIGNMENT

    ASM_PUBLIC(Reset_Handler)
    ASM_PUBLIC(DefaultISR)
    ASM_PUBLIC(SysTick_Handler)
    ASM_PUBLIC(HardFault_Handler)
    ASM_PUBLIC(MemManage_Handler)
    ASM_PUBLIC(BusFault_Handler)
    ASM_PUBLIC(UsageFault_Handler)
    ASM_PUBLIC(DebugMon_Handler)
/*FUNCTION*-------------------------------------------------------------------

 Function Name    : Reset_Handler
 Returned Value   :
 Comments         : startup sequence

 END*-----------------------------------------------------------------------*/

ASM_EQUATE(NVIC_ICER0, 0xE000E180)
ASM_EQUATE(NVIC_ICPR0, 0xE000E280)

    ASM_EXTERN(__iar_program_start)
    ASM_EXTERN(SystemInit)
    ASM_EXTERN(init_data_bss)

    ASM_PUBLIC_BEGIN(Reset_Handler)
    ASM_PUBLIC_FUNC(Reset_Handler)
ASM_LABEL(Reset_Handler)

#if MQX_AUX_CORE
        msr MSP, r0
        isb #15
#endif

        /* Disable interrupts and clear pending flags */
        ldr r0, =NVIC_ICER0
        ldr r1, =NVIC_ICPR0
        ldr r2, =0xFFFFFFFF
        mov r3, #8

ASM_LABEL(_boot_loop)
        cbz r3, _boot_loop_end
        str r2, [r0], #4        /* NVIC_ICERx - clear enable IRQ register */
        str r2, [r1], #4        /* NVIC_ICPRx - clear pending IRQ register */
        sub r3, r3, #1
        b _boot_loop

ASM_LABEL(_boot_loop_end)

        isb #15

        ASM_EXTERN(SystemInit)
        bl ASM_PREFIX(SystemInit)
        ASM_EXTERN(init_data_bss)
        bl ASM_PREFIX(init_data_bss)

#if MQXCFG_ENABLE_FP
        /* turn off fpu register stacking in exception entry */
        ldr r0, =0xE000EF34     /* FPCCR */
        mov r1, #0
        str r1, [r0]
#endif
        ASM_EXTERN(toolchain_startup)
        bl ASM_PREFIX(toolchain_startup)
    ASM_PUBLIC_END(Reset_Handler)

    ASM_EXTERN(_int_kernel_isr)

    ASM_PUBLIC_BEGIN(DefaultISR)
    ASM_PUBLIC_FUNC(DefaultISR)
ASM_LABEL(DefaultISR)
         b ASM_PREFIX(_int_kernel_isr)
    ASM_PUBLIC_END(DefaultISR)

    ASM_PUBLIC_BEGIN(SysTick_Handler)
    ASM_PUBLIC_FUNC(SysTick_Handler)
ASM_LABEL(SysTick_Handler)
         b ASM_PREFIX(_int_kernel_isr)
    ASM_PUBLIC_END(SysTick_Handler)

    ASM_PUBLIC_BEGIN(HardFault_Handler)
    ASM_PUBLIC_FUNC(HardFault_Handler)
ASM_LABEL(HardFault_Handler)
         b ASM_PREFIX(_int_kernel_isr)
    ASM_PUBLIC_END(HardFault_Handler)

    ASM_PUBLIC_BEGIN(MemManage_Handler)
    ASM_PUBLIC_FUNC(MemManage_Handler)
ASM_LABEL(MemManage_Handler)
         b ASM_PREFIX(_int_kernel_isr)
    ASM_PUBLIC_END(MemManage_Handler)

    ASM_PUBLIC_BEGIN(BusFault_Handler)
    ASM_PUBLIC_FUNC(BusFault_Handler)
ASM_LABEL(BusFault_Handler)
         b ASM_PREFIX(_int_kernel_isr)
    ASM_PUBLIC_END(BusFault_Handler)

    ASM_PUBLIC_BEGIN(UsageFault_Handler)
    ASM_PUBLIC_FUNC(UsageFault_Handler)
ASM_LABEL(UsageFault_Handler)
         b ASM_PREFIX(_int_kernel_isr)
    ASM_PUBLIC_END(UsageFault_Handler)

    ASM_PUBLIC_BEGIN(DebugMon_Handler)
    ASM_PUBLIC_FUNC(DebugMon_Handler)
ASM_LABEL(DebugMon_Handler)
         b ASM_PREFIX(_int_kernel_isr)
    ASM_PUBLIC_END(DebugMon_Handler)

        ASM_ALIGN(4)
        ASM_END
