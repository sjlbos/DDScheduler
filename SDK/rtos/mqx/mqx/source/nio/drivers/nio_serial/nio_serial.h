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
* See license agreement file for full license terms including other
* restrictions.
*****************************************************************************
*
* Comments:
*
*
*END************************************************************************/

#ifndef __NIO_SERIAL_H__
#define __NIO_SERIAL_H__

#include <stdint.h>

#include "nio.h"

/* NIO_SERIAL FLAGS */
#define NIO_SERIAL_ABORT            0x00000001      /*!< Transfer abort */
#define NIO_SERIAL_BUFF_OVRFLW      0x00000002      /*!< Buffer overflow flag */
#define NIO_SERIAL_WAITING          0x00000004      /*!< Read/write are waiting on semaphore for receive/free some unites */
#define NIO_SERIAL_FLUSH_OUTPUT     0x00000008      /*!< Flush tx buffer flag */
#define NIO_SERIAL_BUFF_FULL        0x00000010      /*!< Buffer full flag */


#define NIO_SERIAL_MIN_BUFF_SIZE    4               /*!< Minimal size of serial buffers */
#define NIO_SERIAL_BUFF_WM(bufsize) (bufsize/2)     /*!< Buffer */
extern const NIO_DEV_FN_STRUCT nio_serial_dev_fn;

/*! @brief NIO SERIAL parity mode*/
typedef enum _nio_serial_parity_mode_t {
    kNioSerialParityDisabled, /*!< parity disabled */
    kNioSerialParityOdd,      /*!< parity enabled */
    kNioSerialParityEven,     /*!< parity enabled0 */
} nio_serial_parity_mode_t;

/*!
 * @brief Serial number of stop bits.
 *
 * These constants define the number of allowable stop bits to configure.
 */
typedef enum _nio_serial_stop_bit_count {
    kNioSerialOneStopBit,  /*!< one stop bit */
    kNioSerialTwoStopBit,  /*!< two stop bits */
} nio_serial_stop_bit_count_t;

/*! @brief NIO SERIAL module type*/
typedef enum _nio_serial_module_type_t {
    kNioSerialUart,   /*!< UART module is used */
    kNioSerialLpuart, /*!< LPUART module is used */
    kNioSerialLpsci,  /*!< LPSCI module is used */
} nio_serial_module_type_t;

typedef struct
{
    uint32_t SERIAL_INSTANCE;               /*!< Number of used UART peripheral */
    uint32_t BAUDRATE;                      /*!< UART baud rate */
    nio_serial_parity_mode_t PARITY_MODE;   /*!< parity mode, disabled (default), even, odd */
    nio_serial_stop_bit_count_t STOPBIT_COUNT; /*!< number of stop bits, 1 stop bit (default) or 2 stop bits */
    uint8_t BITCOUNT_PERCHAR;               /*!< number of bits, 8-bit (default) or 9-bit in a word (up to 10-bits in some UART instances) */
    uint32_t RXTX_PRIOR;                    /*!< IRQ priority */
    nio_serial_module_type_t MODULE;        /*!< HW module used */
    uint32_t CLK_SOURCE;                    /*!< Clock source for serial module. If serial module doesn't have clock source to choose, leave zero value. */
    uint32_t RX_BUFF_SIZE;                  /*!< Size of rx buffer.*/
    uint32_t TX_BUFF_SIZE;                  /*!< Size of tx buffer.*/
    
} NIO_SERIAL_INIT_DATA_STRUCT;

#endif //__NIO_SERIAL_H__
