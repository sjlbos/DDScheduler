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
*   This file contains the source functions for functions required to
*   specifically initialize the card.
*
*END************************************************************************/

#include <assert.h>
#include "mqx_cnfg.h"
#include "bsp.h"

#if BSPCFG_ENABLE_IO_SUBSYSTEM
#include <fcntl.h>
#include <unistd.h>
#endif

extern void HWTIMER_SYS_SystickIsrAction(void);

#if BSPCFG_ENABLE_IO_SUBSYSTEM
    #include "nio.h"
    #include "nio_serial.h"
    #include "nio_tty.h"
    #include "nio_dummy.h"

    /* Choose default debug serial module */
    #if defined(BOARD_USE_UART)
        #define NIO_SERIAL_DEF_MODULE kNioSerialUart
    #elif defined(BOARD_USE_LPUART)
        #define NIO_SERIAL_DEF_MODULE kNioSerialLpuart
    #elif defined(BOARD_USE_LPSCI)
        #define NIO_SERIAL_DEF_MODULE kNioSerialLpsci
    #else
      #error Default serial module is unsupported or undefined.
    #endif

     /* Provide a default value */
    #ifndef NIO_SERIAL_BUFF_SIZE
       #define NIO_SERIAL_BUFF_SIZE 64
    #endif

    /* Create initialization structure for default serial module */
    const NIO_SERIAL_INIT_DATA_STRUCT nio_serial_default_init =
    {
        .SERIAL_INSTANCE     = BOARD_DEBUG_UART_INSTANCE,
        .BAUDRATE            = BOARD_DEBUG_UART_BAUD,
        .PARITY_MODE         = kNioSerialParityDisabled,
        .STOPBIT_COUNT       = kNioSerialOneStopBit,
        .BITCOUNT_PERCHAR    = 8,
        .RXTX_PRIOR          = 4,
        .MODULE              = NIO_SERIAL_DEF_MODULE,
    /* Check whether individual clock source for debug peripheral is defined */
    #if defined(BOARD_USE_UART) && defined(BOARD_UART_CLOCK_SOURCE)
        .CLK_SOURCE          = BOARD_UART_CLOCK_SOURCE,
    #elif defined(BOARD_USE_LPUART) && defined(BOARD_LPUART_CLOCK_SOURCE)
        .CLK_SOURCE          = BOARD_LPUART_CLOCK_SOURCE,
    #elif defined(BOARD_USE_LPSCI) && defined(BOARD_LPSCI_CLOCK_SOURCE)
        .CLK_SOURCE          = BOARD_LPSCI_CLOCK_SOURCE,
    #else
        .CLK_SOURCE          = 1,
    #endif
        .RX_BUFF_SIZE        = NIO_SERIAL_BUFF_SIZE,
        .TX_BUFF_SIZE        = NIO_SERIAL_BUFF_SIZE,
    };

#endif /* BSPCFG_ENABLE_IO_SUBSYSTEM */

//#if (__MPU_PRESENT == 1)
#ifdef MPU_INSTANCE_COUNT
  #include <fsl_mpu_hal.h>
  #include <fsl_mpu_driver.h>
#endif
//#endif

hwtimer_t systimer;     /* System timer handle */
_mem_pool_id _BSP_sram_pool;
static uint32_t _bsp_get_hwticks(void *param);

/** Workaround for link error message with Keil. It complains that DbgConsole_Init
  * symbol is undefined (referenced by hardware_init.o, even thought it is not used at all
  * and after link processs the DbgConsole_Init is listed in "removed unused symbols".
 */
#ifdef __CC_ARM
#pragma weak DbgConsole_Init
#include "fsl_debug_console.h"
debug_console_status_t DbgConsole_Init(
  uint32_t uartInstance, uint32_t baudRate, debug_console_device_type_t device)
  {return kStatus_DEBUGCONSOLE_InvalidDevice;}
#endif

#ifndef PEX_MQX_KSDK
_WEAK_FUNCTION(void hardware_init(void)) {
    uint8_t i;

    /* enable clock for PORTs */
    for (i = 0; i < PORT_INSTANCE_COUNT; i++)
    {
      CLOCK_SYS_EnablePortClock(i);
    }

    /* Init board clock */
    BOARD_ClockInit();
    /* In case IO sub is turned off, dbg console should be used for printing */
#if !BSPCFG_ENABLE_IO_SUBSYSTEM
    dbg_uart_init();
#endif
}
#endif /* PEX_MQX_KSDK */

#if BSPCFG_ENABLE_IO_SUBSYSTEM
/**FUNCTION********************************************************************
*
* Function Name    : _bsp_nio_stdio_install
* Returned Value   : void
* Comments         :
*   This function is to be called after _nio_init() to install stdio devices
*
*END**************************************************************************/
_WEAK_FUNCTION(void _bsp_nio_stdio_install(void))
{
    void *res;
    int fd;


    /* Install serial driver for default input and output */
    res = _nio_dev_install(BSP_DEFAULT_IO_CHANNEL, &nio_serial_dev_fn, (void*) &nio_serial_default_init, NULL);
    assert(NULL != res);

    /* Install and open dummy drivers for stdin, stdou and stderr. */
    res = _nio_dev_install("dummy:",&nio_dummy_dev_fn, (NULL), NULL);
    assert(NULL != res);

    fd = open("dummy:", 0);  // 0 - stdin
    assert(fd == 0);
    fd = open("dummy:", 0);  // 1 - stdout
    assert(fd == 1);
    fd = open("dummy:", 0);  // 2 - stderr
    assert(fd == 2);

    /* Instal and set tty driver */
    res = _nio_dev_install("tty:", &nio_tty_dev_fn, (void*)&(NIO_TTY_INIT_DATA_STRUCT){BSP_DEFAULT_IO_CHANNEL, 0}, NULL);
    assert(NULL != res);

    close(0);
    fd = open("tty:", NIO_TTY_FLAGS_EOL_RN | NIO_TTY_FLAGS_ECHO);  // 0 - stdin
    assert(fd == 0);

    close(1);
    fd = open("tty:", NIO_TTY_FLAGS_EOL_RN | NIO_TTY_FLAGS_ECHO);  // 1 - stdout
    assert(fd == 1);

    close(2);
    fd = open("tty:", NIO_TTY_FLAGS_EOL_RN | NIO_TTY_FLAGS_ECHO);  // 2 - stderr
    assert(fd == 2);

    /* Dummy driver is not needed any more, therefore should be uninstalled */
    _nio_dev_uninstall("dummy:", NULL);

    /* Avoid warnings on fd and res variables in release targets */
    (void)fd;
    (void)res;
}
#endif /* BSPCFG_ENABLE_IO_SUBSYSTEM */

/*!
* \cond DOXYGEN_PRIVATE
* \brief Pre initialization - initializing requested modules for basic run of MQX.
*/
int _bsp_pre_init(void)
{
    uint32_t result;

/******************************************************************************
         Init gpio platform pins for LEDs, setup board clock source
******************************************************************************/
/* Macro PEX_MQX_KSDK used by PEX team */
#ifndef PEX_MQX_KSDK
    hardware_init();
    /* Configure PINS for default UART instance */
  #if defined(BOARD_USE_LPSCI)
    configure_lpsci_pins(BOARD_DEBUG_UART_INSTANCE);
  #elif defined(BOARD_USE_LPUART)
    configure_lpuart_pins(BOARD_DEBUG_UART_INSTANCE);
  #elif defined(BOARD_USE_UART)
    configure_uart_pins(BOARD_DEBUG_UART_INSTANCE);
  #else
    #error Default serial module is unsupported or undefined.
  #endif
#endif

#if MQX_EXIT_ENABLED
    extern void  _bsp_exit_handler(void);
    /* Set the bsp exit handler, called by _mqx_exit */
    _mqx_set_exit_handler(_bsp_exit_handler);
#endif

    result = _psp_int_init(BSP_FIRST_INTERRUPT_VECTOR_USED, BSP_LAST_INTERRUPT_VECTOR_USED);
    if (result != MQX_OK) {
        return result;
    }


/******************************************************************************
                        Init MQX tick timer
******************************************************************************/
    /* Initialize , set and run system hwtimer */
    result = HWTIMER_SYS_Init(&systimer, &BSP_SYSTIMER_DEV, BSP_SYSTIMER_ID, NULL);
    if (kStatus_OSA_Success != result) {
        return MQX_INVALID_POINTER;
    }
    /* Set isr for timer*/
    if (NULL == OSA_InstallIntHandler(BSP_SYSTIMER_INTERRUPT_VECTOR, HWTIMER_SYS_SystickIsrAction))
    {
        return kHwtimerRegisterHandlerError;
    }
    /* Set interrupt priority */
    NVIC_SetPriority(BSP_SYSTIMER_INTERRUPT_VECTOR, MQX_TO_NVIC_PRIOR(BSP_SYSTIMER_ISR_PRIOR));

    /* Disable  interrupts to ensure ticks are not active until call of _sched_start_internal */
    _int_disable();

    result = HWTIMER_SYS_SetPeriod(&systimer, BSP_ALARM_PERIOD);
    if (kStatus_OSA_Success != result) {
        HWTIMER_SYS_Deinit(&systimer);
        return MQX_INVALID_POINTER;
    }
    result = HWTIMER_SYS_RegisterCallback(&systimer,(hwtimer_callback_t)_time_notify_kernel, NULL);
    if (kStatus_OSA_Success != result) {
        HWTIMER_SYS_Deinit(&systimer);
        return MQX_INVALID_POINTER;
    }
    result = HWTIMER_SYS_Start(&systimer);
    if (kStatus_OSA_Success != result) {
        HWTIMER_SYS_Deinit(&systimer);
        return MQX_INVALID_POINTER;
    }

    /* Initialize the system ticks */
    _time_set_ticks_per_sec(BSP_ALARM_FREQUENCY);
    _time_set_hwticks_per_tick(HWTIMER_SYS_GetModulo(&systimer));
    _time_set_hwtick_function(_bsp_get_hwticks, (void *)NULL);

    return MQX_OK;
}
/*! \endcond */

/*!
 * \cond DOXYGEN_PRIVATE
 * \brief Initialization - called from init task, usually for io initialization.
 */
int _bsp_init(void)
{
    uint32_t result = MQX_OK;

    /** Cache settings **/
    _DCACHE_ENABLE(0);
    _ICACHE_ENABLE(0);

    /* Disable MPU if present on device. This dirty hack is done to workaround missing MPU_DRV_Disable() function in KSDK */
#ifdef MPU_INSTANCE_COUNT
    for(int i = 0; i < MPU_INSTANCE_COUNT; i++)
    {
        MPU_HAL_Disable(g_mpuBase[i]);
    }
#endif

/******************************************************************************
    Install interrupts for UART driver and setup debug console
******************************************************************************/


#if BSPCFG_ENABLE_IO_SUBSYSTEM
    _bsp_nio_stdio_install();
#else /* BSPCFG_ENABLE_IO_SUBSYSTEM */
#if  !defined(PEX_MQX_KSDK)
    dbg_uart_init();
#endif
#endif /* BSPCFG_ENABLE_IO_SUBSYSTEM */

    return result;
}
/*! \endcond */

/**FUNCTION********************************************************************
*
* Function Name    : _bsp_exit_handler
* Returned Value   : void
* Comments         :
*   This function is called when MQX exits
*
*END**************************************************************************/
/*!
 * \cond DOXYGEN_PRIVATE
 */
void _bsp_exit_handler(void)
{
    _DCACHE_DISABLE();
    _ICACHE_DISABLE();
}
/*! \endcond */

/**FUNCTION********************************************************************
 *
 * Function Name    : _bsp_get_hwticks
 * Returned Value   : none
 * Comments         :
 *    This function returns the number of hw ticks that have elapsed
 * since the last interrupt
 *
 *END**************************************************************************/

static uint32_t _bsp_get_hwticks(void *param) {
    hwtimer_time_t time;      //struct for storing time
    HWTIMER_SYS_GetTime(&systimer, &time);
    return time.subTicks;
}
