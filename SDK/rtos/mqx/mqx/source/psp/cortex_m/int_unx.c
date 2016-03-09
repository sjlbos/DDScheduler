
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
* See license agreement file for full license terms including other restrictions.
*****************************************************************************
*
* Comments:
*
*   This file contains the function for the unexpected
*   exception handling function for MQX, which will display on the
*   console what exception has occurred.
*
*   NOTE: the default I/O for the current task is used, since a printf
*   is being done from an ISR.
*   This default I/O must NOT be an interrupt drive I/O channel.
*
*
*END************************************************************************/
#include <stdio.h>
#include "mqx_inc.h"

/*!
 * \brief An MQX-provided default ISR for unhandled interrupts. The function
 * depends on the PSP.
 *
 * The function changes the state of the active task to UNHANDLED_INT_BLOCKED and
 * blocks the task.
 * \n The function uses the default I/O channel to display at least:
 * \li Vector number that caused the unhandled exception.
 * \li Task ID and task descriptor of the active task.
 *
 * \n Depending on the PSP, more information might be displayed.
 *
 * \param[in] parameter Parameter passed to the default ISR.
 *
 * \note
 * Since the ISR uses printf() to display information to the default I/O channel,
 * default I/O must not be on a channel that uses interrupt-driven I/O or the
 * debugger.
 *
 * \warning Blocks the active task.
 *
 * \see _int_install_unexpected_isr
 */
void _int_unexpected_isr
(
    void   *parameter
)
{ /* Body */
    KERNEL_DATA_STRUCT_PTR     kernel_data;
    TD_STRUCT_PTR              td_ptr;

    _GET_KERNEL_DATA(kernel_data);
    td_ptr = kernel_data->ACTIVE_PTR;

#if MQXCFG_PRINT_MEM_DUMP
#error  "MQXCFG_PRINT_MEM_DUMP functionality is currently not supported. This feature will be supported again in future release.\
         Please set MQXCFG_PRINT_MEM_DUMP to zero."
#endif

#if MQXCFG_PRINT_MEM_DUMP
    {
        unsigned int                psp, msp, i;
        printf("\n\r*** UNHANDLED INTERRUPT ***\n\r");
        printf("Vector #: 0x%02x Task Id: 0x%0x Td_ptr 0x%x\n\r",
        (unsigned int)parameter, (unsigned int)td_ptr->TASK_ID, (unsigned int)td_ptr);

        psp = __get_PSP();
        msp = __get_MSP();
        printf("PSP: 0x%08x MSP: 0x%08x PSR: 0x%08x\n\r", psp, msp, (unsigned int)__get_xPSR());

        printf("\n\r\n\rMemory dump:\n\r");
        for (i = 0; i < 32; i += 4) {
            printf("0x%08x : 0x%08x 0x%08x 0x%08x 0x%08x\n\r", psp + i * 4, ((unsigned int*)psp)[i], ((unsigned int*)psp)[i + 1], ((unsigned int*)psp)[i + 2], ((unsigned int*)psp)[i + 3]);
        }

        printf("\n\r\n\rMemory dump:\n\r");
        for (i = 0; i < 32; i += 4) {
            printf("0x%08x : 0x%08x 0x%08x 0x%08x 0x%08x\n\r", msp + i * 4, ((unsigned int*)msp)[i], ((unsigned int*)msp)[i + 1], ((unsigned int*)msp)[i + 2], ((unsigned int*)msp)[i + 3]);
        }
    }
#endif /* MQXCFG_PRINT_MEM_DUMP */
    _INT_DISABLE();
    if (td_ptr->STATE != UNHANDLED_INT_BLOCKED) {
        td_ptr->STATE = UNHANDLED_INT_BLOCKED;
        td_ptr->INFO  = (_mqx_uint)parameter;

        _QUEUE_UNLINK(td_ptr);
    } /* Endif */
   _INT_ENABLE();

} /* Endbody */

/* EOF */
