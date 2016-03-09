/*HEADER**********************************************************************
*
* Copyright 2014 Freescale Semiconductor, Inc.
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
*   This include file is used to provide information needed by
*   an application program using the kernel.
*
*
*END************************************************************************/

#ifndef __bsp_config_h__
#define __bsp_config_h__ 1

/* Define the board type */
//#####################################################################
#define __DEFAULT_PROCESSOR_NUMBER 1
#define __DEFAULT_INTERRUPT_STACK_SIZE 1024

//#####################################################################
/*
** PROCESSOR MEMORY BOUNDS
*/

/* Enable modification of flash configuration bytes during loading for flash targets */
#ifndef BSPCFG_ENABLE_CFMPROTECT
    #define BSPCFG_ENABLE_CFMPROTECT        1
#endif
#if !BSPCFG_ENABLE_CFMPROTECT && defined(__ICCARM__)
    #error Cannot disable CFMPROTECT field on IAR compiler. Please define BSPCFG_ENABLE_CFMPROTECT to 1.
#endif

/* Init startup clock configuration is CPU_CLOCK_CONFIG_0 */

/** MGCT: <category name="BSP Hardware Options"> */

/*
 ** System timer definitions
 */
#define BSP_SYSTIMER_DEV          kSystickDevif
#define BSP_SYSTIMER_ID           0
#define BSP_SYSTIMER_SRC_CLK      kCoreClock
#if PSP_MQX_CPU_IS_ARM_CORTEX_M4
    #define BSP_SYSTIMER_ISR_PRIOR    2
#elif PSP_MQX_CPU_IS_ARM_CORTEX_M0P
    #define BSP_SYSTIMER_ISR_PRIOR    3
#else
    #error Unknown ARM Cortex device
#endif
#define BSP_SYSTIMER_INTERRUPT_VECTOR SysTick_IRQn

/*
 * The clock tick rate in miliseconds - be cautious to keep this value such
 * that it divides 1000 well
 *
 * MGCT: <option type="number" min="1" max="1000"/>
 */
#ifndef BSP_ALARM_FREQUENCY
    #define BSP_ALARM_FREQUENCY             (200)
#endif

/*
* HWTimer period
*/
#ifndef BSP_ALARM_PERIOD
    #define BSP_ALARM_PERIOD             (5000)
#endif


/** MGCT: </category> */

/*
 * Old clock rate definition in MS per tick, kept for compatibility
 */
#define BSP_ALARM_RESOLUTION                (1000 / BSP_ALARM_FREQUENCY)


/****************************************************************************
 *                  DEFAULT MQX INITIALIZATION DEFINITIONS
 * MGCT: <category name="Default MQX initialization parameters">
 */
///#define __KERNEL_DATA_START 0
#define BSP_DEFAULT_START_OF_HEAP                    ((void *)__HEAP_START)
#define BSP_DEFAULT_END_OF_HEAP                      ((void *)__HEAP_END)
#define BSP_DEFAULT_PROCESSOR_NUMBER                 ((uint32_t)__DEFAULT_PROCESSOR_NUMBER)

/* MGCT: <option type="string" quoted="false" allowempty="false"/> */
#ifndef BSP_DEFAULT_INTERRUPT_STACK_SIZE
    #define BSP_DEFAULT_INTERRUPT_STACK_SIZE                  ((uint32_t)__DEFAULT_INTERRUPT_STACK_SIZE)
#endif

/* MGCT: <option type="list">
 * <item name="1 (all levels disabled)" value="(1L)"/>
 * <item name="2" value="(2L)"/>
 * <item name="3" value="(3L)"/>
 * <item name="4" value="(4L)"/>
 * <item name="5" value="(5L)"/>
 * <item name="6" value="(6L)"/>
 * <item name="7" value="(7L)"/>
 * </option>
 */
#ifndef BSP_DEFAULT_MQX_HARDWARE_INTERRUPT_LEVEL_MAX
    #define BSP_DEFAULT_MQX_HARDWARE_INTERRUPT_LEVEL_MAX        (2L)
#endif

/*
 * MGCT: <option type="number"/>
 */
#ifndef BSP_DEFAULT_MAX_MSGPOOLS
    #define BSP_DEFAULT_MAX_MSGPOOLS                            (2L)
#endif

/*
 * MGCT: <option type="number"/>
 */
#ifndef BSP_DEFAULT_MAX_MSGQS
    #define BSP_DEFAULT_MAX_MSGQS                               (16L)
#endif

/*
 * MGCT: <option type="string" maxsize="1024" quoted="false" allowempty="false"/>
 */
#ifndef BSP_DEFAULT_IO_OPEN_MODE
    #define BSP_DEFAULT_IO_OPEN_MODE                      (void *)(NULL)
#endif

/*******************************************************************************
 *                      Ethernet Info
 ******************************************************************************/

#define BSP_ENET_DEVICE_COUNT                   ENET_INSTANCE_COUNT

#ifndef BSP_DEFAULT_ENET_DEVICE
    #define BSP_DEFAULT_ENET_DEVICE             0
#endif

#ifndef BSP_DEFAULT_ENET_OUI
    #define BSP_DEFAULT_ENET_OUI                { 0x00, 0x00, 0x5E, 0, 0, 0 }
#endif

/** MGCT: <category name="BSP Ethernet Options"> */

/*
 * MACNET interrupt levels and vectors
 */
#ifndef BSP_MACNET0_INT_TX_LEVEL
    #define BSP_MACNET0_INT_TX_LEVEL            (4)
#endif
#ifndef BSP_MACNET0_INT_RX_LEVEL
    #define BSP_MACNET0_INT_RX_LEVEL            (4)
#endif
/*
 * The Ethernet PHY device number 0..31
 */
#ifndef BSP_ENET0_PHY_ADDR
    #define BSP_ENET0_PHY_ADDR                  (0)
#endif

/*
** Insertion of IPv4 header checksum by ENET-module.
** MGCT: <option type="bool"/>
*/
#ifndef BSPCFG_ENET_HW_TX_IP_CHECKSUM
    #if HW_ENET_INSTANCE_COUNT
        #define BSPCFG_ENET_HW_TX_IP_CHECKSUM           (1)
    #else
        #define BSPCFG_ENET_HW_TX_IP_CHECKSUM           (0)
    #endif
#endif

/*
** Insertion of protocol checksum by ENET-module.
** MGCT: <option type="bool"/>
*/
#ifndef BSPCFG_ENET_HW_TX_PROTOCOL_CHECKSUM
    #if HW_ENET_INSTANCE_COUNT
        #define BSPCFG_ENET_HW_TX_PROTOCOL_CHECKSUM     (1)
    #else
        #define BSPCFG_ENET_HW_TX_PROTOCOL_CHECKSUM     (0)
    #endif
#endif

/*
** Insertion of IPv4 header checksum by ENET-module.
** MGCT: <option type="bool"/>
*/
#ifndef BSPCFG_ENET_HW_RX_IP_CHECKSUM
    #if HW_ENET_INSTANCE_COUNT
        #define BSPCFG_ENET_HW_RX_IP_CHECKSUM           (1)
    #else
        #define BSPCFG_ENET_HW_RX_IP_CHECKSUM           (0)
    #endif
#endif

/*
** Discard of frames with wrong protocol checksum by ENET-module.
** MGCT: <option type="bool"/>
*/
#ifndef BSPCFG_ENET_HW_RX_PROTOCOL_CHECKSUM
    #if HW_ENET_INSTANCE_COUNT
        #define BSPCFG_ENET_HW_RX_PROTOCOL_CHECKSUM     (1)
    #else
        #define BSPCFG_ENET_HW_RX_PROTOCOL_CHECKSUM     (0)
    #endif
#endif

/*
** Discard of frames with MAC layer errors by ENET-module.
** MGCT: <option type="bool"/>
*/
#ifndef BSPCFG_ENET_HW_RX_MAC_ERR
    #if HW_ENET_INSTANCE_COUNT
        #define BSPCFG_ENET_HW_RX_MAC_ERR               (1)
    #else
        #define BSPCFG_ENET_HW_RX_MAC_ERR               (0)
    #endif
#endif

/** MGCT: </category> */


#endif /* __bsp_config_h__*/
