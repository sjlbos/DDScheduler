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
 *
 *END************************************************************************/

#include <stdarg.h>
#include <stdint.h>
#include <assert.h>
#include <fsl_os_abstraction.h>
/* Required to get info about available serial modules */
#include <fsl_device_registers.h>
/* The only one function from mqx api is _int_install_isr. The reason is better performance and code size. */
#include "mqx.h"
#include "nio_serial.h"
#include "nio.h"
#include "ioctl.h"

#if defined(FSL_FEATURE_UART_FIFO_SIZE)
  #include <fsl_uart_driver.h>
  extern void UART_DRV_IRQHandler(uint32_t instance);
#endif
#if defined(FSL_FEATURE_LPUART_FIFO_SIZE)
  #include <fsl_lpuart_driver.h>
  extern void LPUART_DRV_IRQHandler(uint32_t instance);
#endif
#if defined(FSL_FEATURE_LPSCI_FIFO_SIZE)
  #include <fsl_lpsci_driver.h>
  extern void LPSCI_DRV_IRQHandler(uint32_t instance);
#endif

#if defined ( __IAR_SYSTEMS_ICC__ )
/* MISRA C 2004 rule 20.5 suppress: Error[Pm101]: The error indicator errno shall not be used */
_Pragma ("diag_suppress= Pm101")
#endif

static int nio_serial_open(void *dev_context, const char *dev_name, int flags, void **fp_context, int *error);
static int nio_serial_read(void *dev_context, void *fp_context, void *buf, size_t nbytes, int *error);
static int nio_serial_write(void *dev_context, void *fp_context, const void *buf, size_t nbytes, int *error);
static int nio_serial_ioctl(void *dev_context, void *fp_context, int *error, unsigned long int request, va_list ap);
static int nio_serial_close(void *dev_context, void *fp_context, int *error);
static int nio_serial_init(void *init_data, void **dev_context, int *error);
static int nio_serial_deinit(void *dev_context, int *error);
static int nio_serial_flushBuff(void *dev_struct, int *error);

void nio_serial_uart_rxcallback(uint32_t instance, void * uartState);
void nio_serial_lpuart_rxcallback(uint32_t instance, void * lpuartState);
void nio_serial_lpsci_rxcallback(uint32_t instance, void * lpsciState);

void nio_serial_uart_txcallback(uint32_t instance, void * uartState);
void nio_serial_lpuart_txcallback(uint32_t instance, void * lpuartState);
void nio_serial_lpsci_txcallback(uint32_t instance, void * lpsciState);

const NIO_DEV_FN_STRUCT nio_serial_dev_fn =
{
    .OPEN = nio_serial_open,
    .READ = nio_serial_read,
    .WRITE = nio_serial_write,
    .LSEEK = NULL,
    .IOCTL = nio_serial_ioctl,
    .CLOSE = nio_serial_close,
    .INIT = nio_serial_init,
    .DEINIT = nio_serial_deinit,
};

/*! @brief NIO SERIAL BUFFER */
typedef struct
{
    uint32_t    WRITE_ID;   /*!< write index of buffer */
    uint32_t    READ_ID;    /*!< read index of buffer */
    uint32_t    BUFF_SIZE;  /*!< buffer size */
    uint32_t    FLAGS;      /*!< buffer flags */
    uint8_t     *BUFF;      /*!< pointer to buffer */
    semaphore_t SEMA;       /*!< semaphore instance */
    uint32_t    WAIT_NUM;   /*!< number of unites needs to wait */

} NIO_SERIAL_BUFFER_STRUCT;

typedef struct
{
    mutex_t rlock;
    mutex_t wlock;
    void * serial_state;
    uint32_t instance;
    nio_serial_module_type_t module;
    NIO_SERIAL_BUFFER_STRUCT rxbuffer;
    NIO_SERIAL_BUFFER_STRUCT txbuffer;

} NIO_SERIAL_DEV_CONTEXT_STRUCT;

#if defined(FSL_FEATURE_UART_FIFO_SIZE)
/* Local function to set up UART user config. */
static int get_uart_config (NIO_SERIAL_INIT_DATA_STRUCT *init, uart_user_config_t * uartConfig)
{
    uartConfig->baudRate = init->BAUDRATE;
    switch (init->BITCOUNT_PERCHAR)
    {
      case 8:
        uartConfig->bitCountPerChar = kUart8BitsPerChar;
        break;
      case 9:
        uartConfig->bitCountPerChar = kUart9BitsPerChar;
        break;
      default:
        return -1;
    }
    switch (init->PARITY_MODE)
    {
      case kNioSerialParityDisabled:
        uartConfig->parityMode = kUartParityDisabled;
        break;
      case kNioSerialParityEven:
        uartConfig->parityMode = kUartParityEven;
        break;
      case kNioSerialParityOdd:
        uartConfig->parityMode = kUartParityOdd;
        break;
      default:
        return -1;
    }
    switch (init->STOPBIT_COUNT)
    {
      case kNioSerialOneStopBit:
        uartConfig->stopBitCount = kUartOneStopBit;
        break;
      case kNioSerialTwoStopBit:
        uartConfig->stopBitCount = kUartTwoStopBit;
        break;
      default:
        return -1;
    }

    return 0;
}
#endif
#if defined(FSL_FEATURE_LPUART_FIFO_SIZE)
/* Local function to set up LPUART user config. */
static int get_lpuart_config (NIO_SERIAL_INIT_DATA_STRUCT *init, lpuart_user_config_t * lpuartConfig)
{
    lpuartConfig->clockSource = (clock_lpuart_src_t)init->CLK_SOURCE;
    lpuartConfig->baudRate = init->BAUDRATE;
    switch (init->BITCOUNT_PERCHAR)
    {
      case 8:
        lpuartConfig->bitCountPerChar = kLpuart8BitsPerChar;
        break;
      case 9:
        lpuartConfig->bitCountPerChar = kLpuart9BitsPerChar;
        break;
      case 10:
        lpuartConfig->bitCountPerChar = kLpuart10BitsPerChar;
        break;
      default:
        return -1;
    }
    switch (init->PARITY_MODE)
    {
      case kNioSerialParityDisabled:
        lpuartConfig->parityMode = kLpuartParityDisabled;
        break;
      case kNioSerialParityEven:
        lpuartConfig->parityMode = kLpuartParityEven;
        break;
      case kNioSerialParityOdd:
        lpuartConfig->parityMode = kLpuartParityOdd;
        break;
      default:
        return -1;
    }
    switch (init->STOPBIT_COUNT)
    {
      case kNioSerialOneStopBit:
        lpuartConfig->stopBitCount = kLpuartOneStopBit;
        break;
      case kNioSerialTwoStopBit:
        lpuartConfig->stopBitCount = kLpuartTwoStopBit;
        break;
      default:
        return -1;
    }

    return 0;
}
#endif
#if defined(FSL_FEATURE_LPSCI_FIFO_SIZE)
/* Local function to set up LPSCI user config. */
static int get_lpsci_config (NIO_SERIAL_INIT_DATA_STRUCT *init, lpsci_user_config_t * lpsciConfig)
{
    lpsciConfig->clockSource = (clock_lpsci_src_t)init->CLK_SOURCE;
    lpsciConfig->baudRate = init->BAUDRATE;
    switch (init->BITCOUNT_PERCHAR)
    {
      case 8:
        lpsciConfig->bitCountPerChar = kLpsci8BitsPerChar;
        break;
      case 9:
        lpsciConfig->bitCountPerChar = kLpsci9BitsPerChar;
        break;
      default:
        return -1;
    }
    switch (init->PARITY_MODE)
    {
      case kNioSerialParityDisabled:
        lpsciConfig->parityMode = kLpsciParityDisabled;
        break;
      case kNioSerialParityEven:
        lpsciConfig->parityMode = kLpsciParityEven;
        break;
      case kNioSerialParityOdd:
        lpsciConfig->parityMode = kLpsciParityOdd;
        break;
      default:
        return -1;
    }
    switch (init->STOPBIT_COUNT)
    {
      case kNioSerialOneStopBit:
        lpsciConfig->stopBitCount = kLpsciOneStopBit;
        break;
      case kNioSerialTwoStopBit:
        lpsciConfig->stopBitCount = kLpsciTwoStopBit;
        break;
      default:
        return -1;
    }

    return 0;
}
#endif

static int nio_serial_open(void *dev_context, const char *dev_name, int flags, void **fp_context, int *error)
{
    return 0;
}

static int nio_serial_read(void *dev_context, void *fp_context, void *buf, size_t nbytes, int *error)
{
    assert(dev_context != NULL);
    NIO_SERIAL_DEV_CONTEXT_STRUCT *serial_dev_context = (NIO_SERIAL_DEV_CONTEXT_STRUCT *)dev_context;
    osa_status_t status;
    NIO_SERIAL_BUFFER_STRUCT* rxBuffer = &serial_dev_context->rxbuffer;
    size_t readed = 0;

    if (0 == nbytes)
    {
        return 0;
    }
    if (NULL == buf)
    {
        if (error) {
           *error = NIO_EINVAL;
        }
        return -1;
    }
    status = OSA_MutexLock(&serial_dev_context->rlock, OSA_WAIT_FOREVER);
    if (status != kStatus_OSA_Success)
    {
        if (error) {
            *error = NIO_EIO;
        }
        return -1;
    }

    while (readed < nbytes)
    {   /* Check abort flag */
        if (rxBuffer->FLAGS & NIO_SERIAL_ABORT)
        {   /* Clear abort flag and unlock read mutex */
            rxBuffer->FLAGS &= ~NIO_SERIAL_ABORT;
            status = OSA_MutexUnlock(&serial_dev_context->rlock);
            if (status != kStatus_OSA_Success)
            {
                if (error) {
                    *error = NIO_EIO;
                }
                return -1;
            }
            return readed;
        }
        /* Read data from rx buffer */
        /* If read_id is smaller than write_id (write_id doesn't reach the top),
           just read desired nbytes of available data */
        size_t write_id = rxBuffer->WRITE_ID;
        if (rxBuffer->READ_ID <= write_id)
        {
            size_t left = nbytes - readed;
            size_t rcv_size = write_id - rxBuffer->READ_ID;
            size_t tocopy = (left <= rcv_size) ? left : rcv_size;

            memcpy((void*)((size_t)buf + readed), (const void *)&rxBuffer->BUFF[rxBuffer->READ_ID], tocopy);
            rxBuffer->READ_ID = rxBuffer->READ_ID + tocopy;
            assert(rxBuffer->READ_ID < rxBuffer->BUFF_SIZE);
            readed += tocopy;
        }
        else
        {   /* Read_id is ahead of write_id (write_id has reached the top and read_id has not) */
            size_t left = nbytes - readed;
            size_t tmp;
            size_t tocopy;

            /* Read available data from read_id to the end of the buffer at first */
            tmp = rxBuffer->BUFF_SIZE - rxBuffer->READ_ID;
            tocopy = (left <= tmp)? left : tmp;
            memcpy((void*)((size_t)buf + readed), (const void *)&rxBuffer->BUFF[rxBuffer->READ_ID], tocopy);
            rxBuffer->READ_ID = (rxBuffer->READ_ID + tocopy) % rxBuffer->BUFF_SIZE;
            readed += tocopy;
            left -= tocopy;
            /* If number of the read items is smaller than nbytes, read data from the beginning to write_id */
            if (left)
            {
                tocopy = (left <= write_id)? left: write_id;
                memcpy((void*)((size_t)buf + readed), (const void *)&rxBuffer->BUFF[0], tocopy);
                rxBuffer->READ_ID = tocopy;
                assert(rxBuffer->READ_ID < rxBuffer->BUFF_SIZE);
                readed += tocopy;
            }
        }
        assert(readed <= nbytes);
        if (readed < nbytes)
        {
            rxBuffer->FLAGS |= NIO_SERIAL_WAITING;
            /* Update rxBuffer->WAIT_NUM to avoid buffer overflow from ISR (rx callback)*/
            if ((nbytes - readed) > NIO_SERIAL_BUFF_WM(rxBuffer->BUFF_SIZE))
            {
                rxBuffer->WAIT_NUM = NIO_SERIAL_BUFF_WM(rxBuffer->BUFF_SIZE);
            }
            else
            {
                rxBuffer->WAIT_NUM = (nbytes - readed);
            }
            status = OSA_SemaWait(&rxBuffer->SEMA, OSA_WAIT_FOREVER);
            if (status != kStatus_OSA_Success)
            {
                if (error) {
                    *error = NIO_EIO;
                }
                return -1;
            }
        }
    }   /* while */
    status = OSA_MutexUnlock(&serial_dev_context->rlock);
    if (status != kStatus_OSA_Success)
    {
        if (error) {
            *error = NIO_EIO;
        }
    }
    assert(readed == nbytes);
    return readed;
}

static int nio_serial_write(void *dev_context, void *fp_context, const void *buf, size_t nbytes, int *error)
{
    assert(dev_context != NULL);
    NIO_SERIAL_DEV_CONTEXT_STRUCT *serial_dev_context = (NIO_SERIAL_DEV_CONTEXT_STRUCT *)dev_context;
    NIO_SERIAL_BUFFER_STRUCT* txBuffer = &serial_dev_context->txbuffer;
    size_t written = 0;
    osa_status_t status;

    if (0 == nbytes)
    {
        if (NULL == buf)
        {
            if (0 != nio_serial_flushBuff((void*)serial_dev_context, error))
            {
                return -1;
            }
        }
        return 0;
    }
    if (NULL == buf)
    {
        if (error) {
           *error = NIO_EINVAL;
        }
        return -1;
    }
    status = OSA_MutexLock(&serial_dev_context->wlock, OSA_WAIT_FOREVER);
    if (status != kStatus_OSA_Success)
    {
        if (error) {
            *error = NIO_EIO;
        }
        return -1;
    }

    while (written < nbytes)
    {   /* Check if buffer is full */
        if (txBuffer->FLAGS & NIO_SERIAL_BUFF_FULL)
        {
            txBuffer->FLAGS |= NIO_SERIAL_WAITING;
            /* Update txBuffer->WAIT_NUM to avoid buffer overflow from ISR (tx callback)*/
            if ((nbytes - written) > NIO_SERIAL_BUFF_WM(txBuffer->BUFF_SIZE))
            {
                txBuffer->WAIT_NUM = NIO_SERIAL_BUFF_WM(txBuffer->BUFF_SIZE);
            }
            else
            {
                txBuffer->WAIT_NUM = (nbytes - written);
            }
            /* Wait for place in buffer to send next unites */
            status = OSA_SemaWait(&txBuffer->SEMA, OSA_WAIT_FOREVER);
            if (status != kStatus_OSA_Success)
            {
                if (error) {
                    *error = NIO_EIO;
                }
                return -1;
            }
        }
        size_t read_id = txBuffer->READ_ID;
        if (txBuffer->WRITE_ID < read_id)
        {   /* Read_id is behind write_id. Place to write is just their difference */
            size_t tocopy;
            size_t left = nbytes - written;

            tocopy = read_id - txBuffer->WRITE_ID;
            tocopy = (tocopy <= left) ? tocopy : left;
            memcpy((void*)(&txBuffer->BUFF[txBuffer->WRITE_ID]), (const void *)((size_t)buf + written), tocopy);
            txBuffer->WRITE_ID += tocopy;
            written += tocopy;
        }
        else
        {   /* Read_id is before or the same to write_id. Place to write is from the read_id to the end, and from the beginning to read_id */
            size_t tocopy;
            size_t left = nbytes - written;

            tocopy = txBuffer->BUFF_SIZE - txBuffer->WRITE_ID;
            tocopy = (tocopy <= left) ? tocopy : left;
            memcpy((void*)(&txBuffer->BUFF[txBuffer->WRITE_ID]), (const void *)((size_t)buf + written), tocopy);
            written += tocopy;
            left -= tocopy;
            txBuffer->WRITE_ID = (txBuffer->WRITE_ID + tocopy) % txBuffer->BUFF_SIZE;
            /* If still is something to send, write from the beginning of the buffer */
            if (left)
            {
                tocopy = (left <= read_id)? left : read_id;
                memcpy((void*)(&txBuffer->BUFF[0]), (const void *)((size_t)buf + written), tocopy);
                written += tocopy;
                txBuffer->WRITE_ID = tocopy;
            }
        }
        /* Check if buffer is full, so before next transfer checking buffer's fullness has to occur */
        if (txBuffer->WRITE_ID == txBuffer->READ_ID && txBuffer->READ_ID == read_id)
        {
            txBuffer->FLAGS |= NIO_SERIAL_BUFF_FULL;
        }

        switch (serial_dev_context->module)
        {
            bool res;
#if defined(FSL_FEATURE_UART_FIFO_SIZE)
            case kNioSerialUart:
            {
                uart_state_t* serialState = serial_dev_context->serial_state;			
                _int_disable();
                res = (!serialState->txSize && ((txBuffer->WRITE_ID != txBuffer->READ_ID) || (txBuffer->FLAGS & NIO_SERIAL_BUFF_FULL)));
                _int_enable();
                if (res)
                {
                    UART_DRV_SendData(serial_dev_context->instance, serialState->txBuff, 1);
                }
                break;
            }
#endif
#if defined(FSL_FEATURE_LPUART_FIFO_SIZE)
            case kNioSerialLpuart:
            {
                lpuart_state_t* serialState = serial_dev_context->serial_state;				
                _int_disable();
                res = (!serialState->txSize && ((txBuffer->WRITE_ID != txBuffer->READ_ID) || (txBuffer->FLAGS & NIO_SERIAL_BUFF_FULL)));
                _int_enable();	
                if (res)
                {
                    LPUART_DRV_SendData(serial_dev_context->instance, serialState->txBuff, 1);
                }
                break;
            }
#endif
#if defined(FSL_FEATURE_LPSCI_FIFO_SIZE)
            case kNioSerialLpsci:
            {
                lpsci_state_t* serialState = serial_dev_context->serial_state;
                _int_disable();
                res = (!serialState->txSize && ((txBuffer->WRITE_ID != txBuffer->READ_ID) || (txBuffer->FLAGS & NIO_SERIAL_BUFF_FULL)));
                _int_enable();
                if (res)
                {
                    LPSCI_DRV_SendData(serial_dev_context->instance, serialState->txBuff, 1);
                }
                break;
            }
#endif
            default:
                if (error) {
                    *error = NIO_EINVAL;
                }
                return -1;
        }
    }   /* while */
    assert(written == nbytes);
    status = OSA_MutexUnlock(&serial_dev_context->wlock);
    if (status != kStatus_OSA_Success)
    {
        if (error) {
            *error = NIO_EIO;
        }
        return -1;
    }

    return written;
}


static int nio_serial_ioctl(void *dev_context, void *fp_context, int *error, unsigned long int request, va_list ap)
{
    assert(dev_context != NULL);
    NIO_SERIAL_DEV_CONTEXT_STRUCT *serial_dev_context = (NIO_SERIAL_DEV_CONTEXT_STRUCT *)dev_context;
    uint32_t *dest = NULL;
    size_t buff_id;
    osa_status_t status;

    switch (request) {
        case IOCTL_ABORT:
            /* Unblock waiting tasks for the transfer */
            serial_dev_context->rxbuffer.FLAGS |= NIO_SERIAL_ABORT;
            if (serial_dev_context->rxbuffer.FLAGS & NIO_SERIAL_WAITING)
            {
                serial_dev_context->rxbuffer.FLAGS &= ~NIO_SERIAL_WAITING;
                serial_dev_context->rxbuffer.WAIT_NUM = 0;
                status = OSA_SemaPost(&serial_dev_context->rxbuffer.SEMA);
                if (status != kStatus_OSA_Success)
                {
                    if (error) {
                        *error = NIO_EIO;
                    }
                    return -1;
                }
                /* In this case is NIO_SERIAL_ABORT cleared by nio_serial_read function */
            }
            else
            {   /* No task should be blocked. Clear the abort flag */
                assert(serial_dev_context->rxbuffer.WAIT_NUM == 0);
                serial_dev_context->rxbuffer.FLAGS &= ~NIO_SERIAL_ABORT;
            }
            break;
        case IO_IOCTL_SERIAL_RXBUFF_GET_SIZE:
            dest = va_arg(ap, void*);
            if (NULL == dest)
            {
                goto seterror;
            }
            *dest = serial_dev_context->rxbuffer.BUFF_SIZE;
            break;
        case IO_IOCTL_SERIAL_RXBUFF_RESET:
            _int_disable();
            serial_dev_context->rxbuffer.READ_ID = 0;
            serial_dev_context->rxbuffer.WRITE_ID = 0;
            serial_dev_context->rxbuffer.FLAGS = 0;
            
            switch (serial_dev_context->module)
            {
#if defined(FSL_FEATURE_UART_FIFO_SIZE)
                case kNioSerialUart:
                {
                    uart_state_t* serialState = serial_dev_context->serial_state;
                    serialState->rxBuff = &serial_dev_context->rxbuffer.BUFF[serial_dev_context->rxbuffer.WRITE_ID];
                    break;
                }
#endif
#if defined(FSL_FEATURE_LPUART_FIFO_SIZE)
                case kNioSerialLpuart:
                {
                    lpuart_state_t* serialState = serial_dev_context->serial_state;
                    serialState->rxBuff = &serial_dev_context->rxbuffer.BUFF[serial_dev_context->rxbuffer.WRITE_ID];
                    break;
                }
#endif
#if defined(FSL_FEATURE_LPSCI_FIFO_SIZE)
                case kNioSerialLpsci:
                {
                    lpsci_state_t* serialState = serial_dev_context->serial_state;
                    serialState->rxBuff = &serial_dev_context->rxbuffer.BUFF[serial_dev_context->rxbuffer.WRITE_ID];
                    break;
                }
#endif
            }
            _int_enable();
            break;
        case IO_IOCTL_CHAR_AVAIL:
            dest = va_arg(ap, void*);
            if (NULL == dest)
            {
                goto seterror;
            }
            buff_id = serial_dev_context->rxbuffer.WRITE_ID;
            if (serial_dev_context->rxbuffer.READ_ID <= buff_id)
            {
                *dest = buff_id - serial_dev_context->rxbuffer.READ_ID;
            }
            else
            {
                *dest = (serial_dev_context->rxbuffer.BUFF_SIZE - serial_dev_context->rxbuffer.READ_ID) +
                         buff_id;
            }
            break;
        case IO_IOCTL_SERIAL_RXBUFF_GET_OVERFLOW:
            dest = va_arg(ap, void*);
            if (NULL == dest)
            {
                goto seterror;
            }
            *dest = (bool)(serial_dev_context->rxbuffer.FLAGS & NIO_SERIAL_BUFF_OVRFLW);
            break;
        case IO_IOCTL_SERIAL_RXBUFF_CLR_OVERFLOW:
            dest = va_arg(ap, void*);
            if (NULL == dest)
            {
                goto seterror;
            }
            serial_dev_context->rxbuffer.FLAGS &= ~NIO_SERIAL_BUFF_OVRFLW;
            break;
        case IO_IOCTL_SERIAL_TXBUFF_GET_SIZE:
            dest = va_arg(ap, void*);
            if (NULL == dest)
            {
                goto seterror;
            }
            *dest = serial_dev_context->txbuffer.BUFF_SIZE;
            break;
        case IO_IOCTL_SERIAL_TXBUFF_GET_AVAIL:
            dest = va_arg(ap, void*);
            if (NULL == dest)
            {
                goto seterror;
            }
            buff_id = serial_dev_context->txbuffer.READ_ID;
            if (!(serial_dev_context->txbuffer.FLAGS & NIO_SERIAL_BUFF_FULL))
            {
                *dest = serial_dev_context->txbuffer.BUFF_SIZE;
            }
            else if (serial_dev_context->txbuffer.WRITE_ID <= buff_id)
            {
                *dest = buff_id - serial_dev_context->txbuffer.WRITE_ID;
            }
            else
            {
                *dest = (serial_dev_context->txbuffer.BUFF_SIZE - serial_dev_context->txbuffer.WRITE_ID) + buff_id;
            }
            break;
        case IO_IOCTL_FLUSH_OUTPUT:
            if ( 0 != nio_serial_flushBuff((void*)serial_dev_context, error))
            {
                return -1;
            }
            break;
        default:
            goto seterror;
    }
    return 0;

seterror:
    if (error) {
        *error = NIO_EINVAL;
    }
    return -1;
}

static int nio_serial_flushBuff(void *dev_struct, int *error)
{
    NIO_SERIAL_DEV_CONTEXT_STRUCT *serial_dev = (NIO_SERIAL_DEV_CONTEXT_STRUCT*)dev_struct;
    osa_status_t status;

    assert(serial_dev != NULL);
    status = OSA_MutexLock(&serial_dev->wlock, OSA_WAIT_FOREVER);
    if (status != kStatus_OSA_Success)
    {
        if (error) {
            *error = NIO_EIO;
        }
        return -1;
    }
    /* Check if buffer is not empty */
    if ((serial_dev->txbuffer.READ_ID != serial_dev->txbuffer.WRITE_ID) ||
        (serial_dev->txbuffer.FLAGS & NIO_SERIAL_BUFF_FULL))
    {   /* Buffer is not empty. Flush data */
        serial_dev->txbuffer.FLAGS |= NIO_SERIAL_FLUSH_OUTPUT;
        status = OSA_SemaWait(&serial_dev->txbuffer.SEMA, OSA_WAIT_FOREVER);
        if (status != kStatus_OSA_Success)
        {
            if (error) {
                *error = NIO_EIO;
            }
            return -1;
        }
        serial_dev->txbuffer.FLAGS &= ~NIO_SERIAL_FLUSH_OUTPUT;
    }
    status = OSA_MutexUnlock(&serial_dev->wlock);
    if (status != kStatus_OSA_Success)
    {
        if (error) {
            *error = NIO_EIO;
        }
        return -1;
    }
    return 0;
}

static int nio_serial_close(void *dev_context, void *fp_context, int *error)
{
    return 0;
}

static int nio_serial_init(void *init_data, void **dev_context, int *error)
{
    assert(init_data != NULL);
    assert(dev_context != NULL);
    NIO_SERIAL_DEV_CONTEXT_STRUCT *serial_dev_context = (NIO_SERIAL_DEV_CONTEXT_STRUCT *)dev_context;
    NIO_SERIAL_INIT_DATA_STRUCT *init = (NIO_SERIAL_INIT_DATA_STRUCT*)init_data;
    NIO_SERIAL_BUFFER_STRUCT *rxBuffer = NULL;
    NIO_SERIAL_BUFFER_STRUCT *txBuffer = NULL;
    int32_t IRQNumber;
    int result;

#if defined(FSL_FEATURE_UART_FIFO_SIZE)
    assert((init->MODULE == kNioSerialUart)? (init->SERIAL_INSTANCE < UART_INSTANCE_COUNT) : 1);
#endif
#if defined(FSL_FEATURE_LPUART_FIFO_SIZE)
    assert((init->MODULE == kNioSerialLpuart)? (init->SERIAL_INSTANCE < LPUART_INSTANCE_COUNT) : 1);
#endif

    serial_dev_context = (NIO_SERIAL_DEV_CONTEXT_STRUCT*) OSA_MemAlloc(sizeof(NIO_SERIAL_DEV_CONTEXT_STRUCT));
    if (NULL == serial_dev_context)
    {
        result = NIO_ENOMEM;
        goto cleanupA;
    }
    rxBuffer = &serial_dev_context->rxbuffer;
    /* Default initialization of RX buffer */
    rxBuffer->BUFF_SIZE = init->RX_BUFF_SIZE;
    rxBuffer->READ_ID = 0;
    rxBuffer->WRITE_ID = 0;
    rxBuffer->FLAGS = 0;
    rxBuffer->WAIT_NUM = 0;
    /* Check whether at least the minimum serial buffer size was set */
    if (NIO_SERIAL_MIN_BUFF_SIZE > rxBuffer->BUFF_SIZE)
    {
        rxBuffer->BUFF_SIZE = NIO_SERIAL_MIN_BUFF_SIZE;
    }
    /* SERIAL Rx buffer initialization */
    rxBuffer->BUFF = (uint8_t*)OSA_MemAlloc(rxBuffer->BUFF_SIZE);
    if (NULL == rxBuffer->BUFF)
    {
        result = NIO_ENOMEM;
        goto cleanupB;
    }

    txBuffer = &serial_dev_context->txbuffer;
    /* Default initialization of RX buffer */
    txBuffer->BUFF_SIZE = init->TX_BUFF_SIZE;
    txBuffer->READ_ID = 0;
    txBuffer->WRITE_ID = 0;
    txBuffer->FLAGS = 0;
    txBuffer->WAIT_NUM = 0;

    /* Check whether at least the minimum serial buffer size was set */
    if (NIO_SERIAL_MIN_BUFF_SIZE > txBuffer->BUFF_SIZE)
    {
        txBuffer->BUFF_SIZE = NIO_SERIAL_MIN_BUFF_SIZE;
    }
    /* SERIAL Tx buffer initialization */
    txBuffer->BUFF = (uint8_t*)OSA_MemAlloc(txBuffer->BUFF_SIZE);
    if (NULL == txBuffer->BUFF)
    {
        result = NIO_ENOMEM;
        goto cleanupC;
    }
    /* Synchronization objects initialization */
    if ( kStatus_OSA_Success != OSA_MutexCreate(&serial_dev_context->rlock))
    {
        result = NIO_ENOMEM;
        goto cleanupD;
    }
    if ( kStatus_OSA_Success !=OSA_MutexCreate(&serial_dev_context->wlock))
    {
        result = NIO_ENOMEM;
        goto cleanupE;
    }
    if( kStatus_OSA_Success != OSA_SemaCreate(&rxBuffer->SEMA, 0))
    {
        result = NIO_ENOMEM;
        goto cleanupF;
    }
    if( kStatus_OSA_Success != OSA_SemaCreate(&txBuffer->SEMA, 0))
    {
        result = NIO_ENOMEM;
        goto cleanupG;
    }

    switch (init->MODULE)
    {
#if defined(FSL_FEATURE_UART_FIFO_SIZE)
        case kNioSerialUart:
        {
            uart_user_config_t uartConfig;
            if (0 != get_uart_config(init_data, &uartConfig))
            {
                result = NIO_ENXIO;
                goto cleanupH;
            }
            serial_dev_context->serial_state = OSA_MemAlloc(sizeof(uart_state_t));
            if (NULL == serial_dev_context->serial_state)
            {
                result = NIO_ENOMEM;
                goto cleanupH;
            }
            /* SERIAL handler interrupt installation */
            IRQNumber = g_uartRxTxIrqId[init->SERIAL_INSTANCE];
            NVIC_SetPriority((IRQn_Type)IRQNumber, init->RXTX_PRIOR);
            _int_install_isr(IRQNumber, (INT_ISR_FPTR)UART_DRV_IRQHandler, (void*)init->SERIAL_INSTANCE);
            /* SDK HAL init */
            if ( kStatus_UART_Success != UART_DRV_Init(init->SERIAL_INSTANCE, (uart_state_t *) serial_dev_context->serial_state, &uartConfig))
            {
                result = NIO_ENXIO;
                goto cleanupI;
            }
            /* Install callback function to handle RX buffer */
            UART_DRV_InstallRxCallback(init->SERIAL_INSTANCE, &nio_serial_uart_rxcallback, rxBuffer->BUFF, rxBuffer, true);
            /* Install callback function to handle TX buffer */
            UART_DRV_InstallTxCallback(init->SERIAL_INSTANCE, &nio_serial_uart_txcallback, txBuffer->BUFF, txBuffer);
            break;
        }
#endif
#if defined(FSL_FEATURE_LPUART_FIFO_SIZE)
        case kNioSerialLpuart:
        {
            lpuart_user_config_t lpuartConfig;
            if (0 != get_lpuart_config(init_data, &lpuartConfig))
            {
                result = NIO_ENXIO;
                goto cleanupH;
            }
            serial_dev_context->serial_state = OSA_MemAlloc(sizeof(lpuart_state_t));
            if (NULL == serial_dev_context->serial_state)
            {
                result = NIO_ENOMEM;
                goto cleanupH;
            }
            /* SERIAL handler interrupt installation */
            IRQNumber = g_lpuartRxTxIrqId[init->SERIAL_INSTANCE];
            NVIC_SetPriority((IRQn_Type)IRQNumber, init->RXTX_PRIOR);
            _int_install_isr(IRQNumber, (INT_ISR_FPTR)LPUART_DRV_IRQHandler, (void*)init->SERIAL_INSTANCE);
            /* SDK HAL init */
            if ( kStatus_LPUART_Success != LPUART_DRV_Init(init->SERIAL_INSTANCE, (lpuart_state_t *) serial_dev_context->serial_state, &lpuartConfig))
            {
                result = NIO_ENXIO;
                goto cleanupI;
            }
            /* Install callback function to handle RX buffer */
            LPUART_DRV_InstallRxCallback(init->SERIAL_INSTANCE, &nio_serial_lpuart_rxcallback, rxBuffer->BUFF, rxBuffer, true);
            /* Install callback function to handle TX buffer */
            LPUART_DRV_InstallTxCallback(init->SERIAL_INSTANCE, &nio_serial_lpuart_txcallback, txBuffer->BUFF, txBuffer);
            break;
        }
#endif
#if defined(FSL_FEATURE_LPSCI_FIFO_SIZE)
        case kNioSerialLpsci:
        {
            lpsci_user_config_t lpsciConfig;
            if (0 != get_lpsci_config(init_data, &lpsciConfig))
            {
                result = NIO_ENXIO;
                goto cleanupH;
            }
            serial_dev_context->serial_state = OSA_MemAlloc(sizeof(lpsci_state_t));
            if (NULL == serial_dev_context->serial_state)
            {
                result = NIO_ENOMEM;
                goto cleanupH;
            }
            /* SERIAL handler interrupt installation */
            IRQNumber = g_lpsciRxTxIrqId[init->SERIAL_INSTANCE];
            NVIC_SetPriority((IRQn_Type)IRQNumber, init->RXTX_PRIOR);
            _int_install_isr(IRQNumber, (INT_ISR_FPTR)LPSCI_DRV_IRQHandler, (void*)init->SERIAL_INSTANCE);
            /* SDK HAL init */
            if ( kStatus_LPSCI_Success != LPSCI_DRV_Init(init->SERIAL_INSTANCE, (lpsci_state_t *) serial_dev_context->serial_state, &lpsciConfig))
            {
                result = NIO_ENXIO;
                goto cleanupI;
            }
            /* Install callback function to handle RX buffer */
            LPSCI_DRV_InstallRxCallback(init->SERIAL_INSTANCE, &nio_serial_lpsci_rxcallback, rxBuffer->BUFF, rxBuffer, true);
            /* Install callback function to handle TX buffer */
            LPSCI_DRV_InstallTxCallback(init->SERIAL_INSTANCE, &nio_serial_lpsci_txcallback, txBuffer->BUFF, txBuffer);
            break;
        }
#endif
        default:
            result = NIO_ENXIO;
            goto cleanupH;
    }
    /* Device context initialization */
    serial_dev_context->module = init->MODULE;
    serial_dev_context->instance = init->SERIAL_INSTANCE;
    *dev_context = (void*)serial_dev_context;
    return 0;

/* Cleanup part called after some failure */
cleanupI:
    OSA_MemFree(serial_dev_context->serial_state);
cleanupH:
    OSA_SemaDestroy(&serial_dev_context->txbuffer.SEMA);
cleanupG:
    OSA_SemaDestroy(&serial_dev_context->rxbuffer.SEMA);
cleanupF:
    OSA_MutexDestroy(&serial_dev_context->wlock);
cleanupE:
    OSA_MutexDestroy(&serial_dev_context->rlock);
cleanupD:
    OSA_MemFree(txBuffer->BUFF);
cleanupC:
    OSA_MemFree(rxBuffer->BUFF);
cleanupB:
    OSA_MemFree(serial_dev_context);
cleanupA:
    assert(0 != result);
    if (error) {
        *error = result;
    }
    return -1;
}

static int nio_serial_deinit(void *dev_context, int *error)
{
    NIO_SERIAL_DEV_CONTEXT_STRUCT *serial_dev_context = (NIO_SERIAL_DEV_CONTEXT_STRUCT *)dev_context;

    if (NULL == dev_context)
    {
        if (error) {
            *error = NIO_EINVAL;
        }
        return -1;
    }

    switch (serial_dev_context->module)
    {
#if defined(FSL_FEATURE_UART_FIFO_SIZE)
      case kNioSerialUart:
          UART_DRV_Deinit(serial_dev_context->instance);
          break;
#endif
#if defined(FSL_FEATURE_LPUART_FIFO_SIZE)
      case kNioSerialLpuart:
          LPUART_DRV_Deinit(serial_dev_context->instance);
          break;
#endif
#if defined(FSL_FEATURE_LPSCI_FIFO_SIZE)
      case kNioSerialLpsci:
          LPSCI_DRV_Deinit(serial_dev_context->instance);
          break;
#endif
      default:
          if (error) {
              *error = NIO_EINVAL;
          }
        return -1;
    }
    assert(&serial_dev_context->rlock);
    assert(&serial_dev_context->wlock);
    assert(&serial_dev_context->rxbuffer.SEMA);
    assert(&serial_dev_context->txbuffer.SEMA);
    assert(serial_dev_context->serial_state);
    assert(serial_dev_context->rxbuffer.BUFF);
    assert(serial_dev_context->txbuffer.BUFF);
    assert(dev_context);

    OSA_MutexDestroy(&serial_dev_context->rlock);
    OSA_MutexDestroy(&serial_dev_context->wlock);
    OSA_SemaDestroy(&serial_dev_context->rxbuffer.SEMA);
    OSA_SemaDestroy(&serial_dev_context->txbuffer.SEMA);
    OSA_MemFree(serial_dev_context->serial_state);
    OSA_MemFree((NIO_SERIAL_BUFFER_STRUCT*)(&serial_dev_context->rxbuffer)->BUFF);
    OSA_MemFree((NIO_SERIAL_BUFFER_STRUCT*)(&serial_dev_context->txbuffer)->BUFF);
    OSA_MemFree(dev_context);
    return 0;
}

/* Rx callback function to handle logic of reading hw fifo */
static inline void nio_serial_rxcallback(NIO_SERIAL_BUFFER_STRUCT *buffer, uint8_t **dest)
{
    assert(NULL != buffer);
    assert(NULL != dest);
    /* Check whether next transfer will cause buffer overflow */
    if (((buffer->WRITE_ID + 1) % buffer->BUFF_SIZE) == buffer->READ_ID)
    {   /* If overflow, set the error and overwrite actual buffer position until
           read_id will be changed */
        buffer->FLAGS |= NIO_SERIAL_BUFF_OVRFLW;
        return;
    }
    /* Increase buffer write_id */
    buffer->WRITE_ID = (buffer->WRITE_ID + 1) % buffer->BUFF_SIZE;
    /* Set new destination to store next received item */
    *dest = &buffer->BUFF[buffer->WRITE_ID];
    /* Check if some transmission is waiting for next unites */
    if ((buffer->FLAGS & NIO_SERIAL_WAITING) && !(buffer->FLAGS & NIO_SERIAL_ABORT))
    {   /* If desired unites were received, post blocking semaphore */
        if (!(--buffer->WAIT_NUM))
        {
            osa_status_t status = kStatus_OSA_Success;
            /* disable 'unused variable' warning */
            (void)status;
            buffer->FLAGS &= ~NIO_SERIAL_WAITING;
            status = OSA_SemaPost(&buffer->SEMA);
            /* Should always pass */
            assert(kStatus_OSA_Success == status);
        }
    }
    return;
}

/* Callback functions to handle rxBuffer */
#if defined(FSL_FEATURE_UART_FIFO_SIZE)
void nio_serial_uart_rxcallback(uint32_t instance, void * serialState)
{
    uart_state_t* serial_dev_context = (uart_state_t*)serialState;
    NIO_SERIAL_BUFFER_STRUCT *rxBuffer = (NIO_SERIAL_BUFFER_STRUCT*)serial_dev_context->rxCallbackParam;
    nio_serial_rxcallback(rxBuffer, &serial_dev_context->rxBuff);
    return;
}
#endif

#if defined(FSL_FEATURE_LPUART_FIFO_SIZE)
void nio_serial_lpuart_rxcallback(uint32_t instance, void * serialState)
{
    lpuart_state_t* serial_dev_context = (lpuart_state_t*)serialState;
    NIO_SERIAL_BUFFER_STRUCT *rxBuffer = (NIO_SERIAL_BUFFER_STRUCT*)serial_dev_context->rxCallbackParam;
    nio_serial_rxcallback(rxBuffer, &serial_dev_context->rxBuff);
    return;
}
#endif

#if defined(FSL_FEATURE_LPSCI_FIFO_SIZE)
void nio_serial_lpsci_rxcallback(uint32_t instance, void * serialState)
{
    lpsci_state_t* serial_dev_context = (lpsci_state_t*)serialState;
    NIO_SERIAL_BUFFER_STRUCT *rxBuffer = (NIO_SERIAL_BUFFER_STRUCT*)serial_dev_context->rxCallbackParam;
    nio_serial_rxcallback(rxBuffer, &serial_dev_context->rxBuff);
    return;
}
#endif

/* Tx callback function to handle logic of filling hw fifo */
static inline int nio_serial_txcallback(NIO_SERIAL_BUFFER_STRUCT *buffer, const uint8_t **src)
{
    /* Increase buffer read_id */
    buffer->READ_ID = (buffer->READ_ID + 1) % buffer->BUFF_SIZE;
    /* Set new source for transmitting next item */
    *src = &buffer->BUFF[buffer->READ_ID];
    /* Clear buffer full flag */
    buffer->FLAGS &= ~NIO_SERIAL_BUFF_FULL;
    /* Check if some transmission is waiting to write next unites */
    if (buffer->FLAGS & NIO_SERIAL_WAITING)
    {   /* If the desired place in buffer was released, post blocking semaphore */
        if (!(--buffer->WAIT_NUM))
        {
            osa_status_t status = kStatus_OSA_Success;
            /* disable 'unused variable' warning */
            (void)status;
            buffer->FLAGS &= ~NIO_SERIAL_WAITING;
            status = OSA_SemaPost(&buffer->SEMA);
            /* Should always pass */
            assert(status == kStatus_OSA_Success);
        }
    }
    /* Check whether is something to send in buffer */
    if (buffer->READ_ID == buffer->WRITE_ID)
    {   /* Post waiting task for buffer flush */
        if (buffer->FLAGS & NIO_SERIAL_FLUSH_OUTPUT)
        {
            osa_status_t status = kStatus_OSA_Success;
            /* disable 'unused variable' warning */
            (void)status;
            status = OSA_SemaPost(&buffer->SEMA);
            /* Should always pass */
            assert(status == kStatus_OSA_Success);
        }
        /* stop the transfer, nothing to send */
        return 1;
    }
    return 0;
}

/* Callback functions to handle txbuffer */
#if defined(FSL_FEATURE_UART_FIFO_SIZE)
void nio_serial_uart_txcallback(uint32_t instance, void * serialState)
{
    uart_state_t* serial_dev_context = (uart_state_t*)serialState;
    NIO_SERIAL_BUFFER_STRUCT *txBuffer = (NIO_SERIAL_BUFFER_STRUCT*)serial_dev_context->txCallbackParam;

    if (nio_serial_txcallback(txBuffer, &serial_dev_context->txBuff))
    {   /* Stop the transfer */
        serial_dev_context->txSize = 0;
    }
    return;
}
#endif

#if defined(FSL_FEATURE_LPUART_FIFO_SIZE)
void nio_serial_lpuart_txcallback(uint32_t instance, void * serialState)
{
    lpuart_state_t* serial_dev_context = (lpuart_state_t*)serialState;
    NIO_SERIAL_BUFFER_STRUCT *txBuffer = (NIO_SERIAL_BUFFER_STRUCT*)serial_dev_context->txCallbackParam;

    if (nio_serial_txcallback(txBuffer, &serial_dev_context->txBuff))
    {   /* Stop the transfer */
        serial_dev_context->txSize = 0;
    }
    return;
}
#endif

#if defined(FSL_FEATURE_LPSCI_FIFO_SIZE)
void nio_serial_lpsci_txcallback(uint32_t instance, void * serialState)
{
    lpsci_state_t* serial_dev_context = (lpsci_state_t*)serialState;
    NIO_SERIAL_BUFFER_STRUCT *txBuffer = (NIO_SERIAL_BUFFER_STRUCT*)serial_dev_context->txCallbackParam;

    if (nio_serial_txcallback(txBuffer, &serial_dev_context->txBuff))
    {   /* Stop the transfer */
        serial_dev_context->txSize = 0;
    }
    return;
}
#endif
