/*HEADER**********************************************************************
*
* Copyright 2010 Freescale Semiconductor, Inc.
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
*   This file contains the type definitions for the Kinetis microcontrollers.
*
*
*END************************************************************************/

#ifndef __kinetis_h__
#define __kinetis_h__

#ifndef __ASM__

    #include <mqx.h>
/* Include FSK device registers from Platform SDK */
    #include "fsl_device_registers.h"
    #include "nvic.h"
    #include "kinetis_mpu.h"
#endif /* __ASM__ */

#ifdef __cplusplus
extern "C" {
#endif

/*
*******************************************************************************
**
**                 LINKER SYMBOLS ACCESS
**
*******************************************************************************
*/

/* Addresses for VECTOR_TABLE and VECTOR_RAM come from the linker file */
#if defined(__CC_ARM) /* Keil MDK */
    extern uint32_t Image$$ARM_LIB_HEAP$$Base[];
    extern uint32_t Image$$ARM_LIB_STACK$$ZI$$Base[];
    extern uint32_t Image$$VECTOR_ROM$$Base[];
    extern uint32_t Image$$VECTOR_RAM$$Base[];
    #define __VECTOR_TABLE Image$$VECTOR_ROM$$Base
    #define __VECTOR_RAM Image$$VECTOR_RAM$$Base
    #define __HEAP_START Image$$ARM_LIB_HEAP$$Base
    #define __HEAP_END Image$$ARM_LIB_STACK$$ZI$$Base  /* Segment is defined in opposite way (from higher address to lower) than usually!!!  */
#elif defined(__ICCARM__) /* IAR */
    #pragma section = "ZI"
    #define __ZI_END  (__section_end("ZI"))
    #pragma section = "RW"
    #define __RW_END  (__section_end("RW"))
    #pragma section = "HEAP"
    #define __HEAP_START  (__section_begin("HEAP"))
    #pragma section = "CSTACK"
    #define __HEAP_END  (__section_begin("CSTACK"))
    extern uint32_t __VECTOR_TABLE[NUMBER_OF_INT_VECTORS * sizeof(uint32_t)];
    extern uint32_t __VECTOR_RAM[NUMBER_OF_INT_VECTORS * sizeof(uint32_t)];
#elif defined(__GNUC__) /* GCC */
    extern uint32_t __bss_end__[];
    extern uint32_t __StackLimit[];
    #define __HEAP_START __bss_end__
    #define __HEAP_END  __StackLimit
    extern uint32_t __VECTOR_TABLE[NUMBER_OF_INT_VECTORS * sizeof(uint32_t)];
    extern uint32_t __VECTOR_RAM[NUMBER_OF_INT_VECTORS * sizeof(uint32_t)];
#else
    #error Unsupported tool
#endif

/*
*******************************************************************************
**
**                  CONSTANT DEFINITIONS
**
*******************************************************************************
*/

/* flash swap default value */
#ifndef PSP_HAS_FLASH_SWAP
#define PSP_HAS_FLASH_SWAP                      0
#endif

/* Cache and MMU definition values */
#ifndef PSP_HAS_MMU
#define PSP_HAS_MMU                             0
#endif

#ifndef PSP_HAS_CODE_CACHE
#define PSP_HAS_CODE_CACHE                      0
#endif

#ifndef PSP_HAS_DATA_CACHE
#define PSP_HAS_DATA_CACHE                      0
#endif

#ifndef __ASM__
#if MQXCFG_ENABLE_FP && (__FPU_PRESENT == 0)
  #error FP is enabled but FPU does not exist on this processor.
#endif

#if MQX_SAVE_FP_ALWAYS && (MQXCFG_ENABLE_FP == 0)
  #error FP saving always is enabled but FPU is disabled.
#endif

#if MQX_SAVE_FP_ALWAYS && (__FPU_PRESENT == 0)
  #error FP saving always is enabled but FPU does not exist on this processor.
#endif
#endif /* __ASM__ */

#ifndef PSP_BYPASS_P3_WFI
#define PSP_BYPASS_P3_WFI                       0
#endif

#define PSP_CACHE_LINE_SIZE                     (0x10)

#ifndef __ASM__

/* Standard cache macros */
#if PSP_HAS_DATA_CACHE
#define _DCACHE_ENABLE(n)               _dcache_enable()
#define _DCACHE_DISABLE()               _dcache_disable()
#define _DCACHE_FLUSH()                 _dcache_flush()
#define _DCACHE_FLUSH_LINE(p)           _dcache_flush_line(p)
#define _DCACHE_FLUSH_MLINES(p, m)      _dcache_flush_mlines(p,m)
#define _DCACHE_INVALIDATE()            _dcache_invalidate()
#define _DCACHE_INVALIDATE_LINE(p)      _dcache_invalidate_line(p)
#define _DCACHE_INVALIDATE_MLINES(p, m) _dcache_invalidate_mlines(p, m)

#define _DCACHE_FLUSH_MBYTES(p, m)             _DCACHE_FLUSH_MLINES(p, m)
#define _DCACHE_INVALIDATE_MBYTES(p, m)        _DCACHE_INVALIDATE_MLINES(p, m)

#else
#define _DCACHE_ENABLE(n)
#define _DCACHE_DISABLE()
#define _DCACHE_FLUSH()
#define _DCACHE_FLUSH_LINE(p)
#define _DCACHE_FLUSH_MLINES(p, m)
#define _DCACHE_INVALIDATE()
#define _DCACHE_INVALIDATE_LINE(p)
#define _DCACHE_INVALIDATE_MLINES(p, m)

#define _DCACHE_FLUSH_MBYTES(p, m)
#define _DCACHE_INVALIDATE_MBYTES(p, m)
#endif

#if PSP_HAS_CODE_CACHE
#define _ICACHE_ENABLE(n)                   _icache_enable()
#define _ICACHE_DISABLE()                   _icache_disable()
#define _ICACHE_INVALIDATE()                _icache_invalidate()
#define _ICACHE_INVALIDATE_LINE(p)          _icache_invalidate_line(p)
#define _ICACHE_INVALIDATE_MLINES(p, m)     _icache_invalidate_mlines(p, m)
#else
#define _ICACHE_ENABLE(n)
#define _ICACHE_DISABLE()
#define _ICACHE_FLUSH()
#define _ICACHE_FLUSH_LINE(p)
#define _ICACHE_FLUSH_MLINES(p, m)
#define _ICACHE_INVALIDATE()
#define _ICACHE_INVALIDATE_LINE(p)
#define _ICACHE_INVALIDATE_MLINES(p, m)
#endif

/** Sleep function definition.
 */
#if PSP_BYPASS_P3_WFI
#define _ASM_SLEEP(param)    {extern void(*_sleep_p3_ram)(uint32_t*);_sleep_p3_ram(param);}
#else
#define _ASM_SLEEP(param)    _ASM_WFI()
#endif

/* Each PSP defines its own interrupt type. makes it hard to share drivers.
correct is PSP_INTERRUPT_TABLE_INDEX and member of this enum should be interpreted as int */
#define PSP_INTERRUPT_TABLE_INDEX               IRQn_Type
/*compatibility typedef because of new IO maps (SDK)*/
typedef IRQn_Type IRQInterruptIndex;


/*
*******************************************************************************
**
**              FUNCTION PROTOTYPES AND GLOBAL EXTERNS
**
*******************************************************************************
*/

#define _psp_mem_check_access(addr, size, flags)    \
                    _kinetis_mpu_sw_check(addr, size, flags)

#define _psp_mem_check_access_mask(addr, size, flags, mask) \
                    _kinetis_mpu_sw_check_mask(addr, size, flags, mask)

#define _psp_mpu_add_region(start, end, flags)  \
                    _kinetis_mpu_add_region(start, end, flags)

/* PSP Cache prototypes */
void _dcache_enable(void);
void _dcache_disable(void);
void _dcache_flush(void);
void _dcache_flush_line(void *);
void _dcache_flush_mlines(void *, uint32_t);
void _dcache_invalidate(void);
void _dcache_invalidate_line(void *);
void _dcache_invalidate_mlines(void *, uint32_t);

void _icache_enable(void);
void _icache_disable(void);
void _icache_flush(void);
void _icache_flush_line(void *);
void _icache_flush_mlines(void *, uint32_t);
void _icache_invalidate(void);
void _icache_invalidate_line(void *);
void _icache_invalidate_mlines(void *, uint32_t);

#endif /* __ASM__ */

#ifdef __cplusplus
}
#endif

#endif /* __kinetis_h__ */
