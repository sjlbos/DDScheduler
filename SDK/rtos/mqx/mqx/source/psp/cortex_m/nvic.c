
/*HEADER**********************************************************************
*
* Copyright 2012 Freescale Semiconductor, Inc.
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
* ARM Nested Vectored Interrupt Controller (NVIC)
*
*
*END************************************************************************/

#include "mqx_inc.h"
#include "nvic.h"

/*!
 * \brief 	Initialize a specific interrupt in the cortex core nvic
 * 
 * \param[in] irq Interrupt number
 * \param[in] prior Interrupt priority 
 * \param[in] enable enable the interrupt now?
 *
 * \return uint32_t MQX_OK or error code
 */
_mqx_uint _nvic_int_init
    (
        /* [IN] Interrupt number */
        _mqx_uint irq,

        /* [IN] Interrupt priority */
        _mqx_uint prior,

        /* [IN] enable the interrupt now?  */
        bool enable
    )
{
    if (prior >= (1 << CORTEX_PRIOR_IMPL)) {
        return MQX_INVALID_PARAMETER;
    }

    if (irq >= PSP_INT_FIRST_INTERNAL && irq <= PSP_INT_LAST_INTERNAL) {
        NVIC_SetPriority ((IRQn_Type)irq, (uint32_t)prior);

        if (enable)
            NVIC_EnableIRQ((IRQn_Type)irq);
        else
            NVIC_DisableIRQ((IRQn_Type)irq);
    }
    else
        return MQX_INVALID_PARAMETER;

    return MQX_OK;
}

/*!
 * \brief Enable interrupt on cortex core NVIC
 * 
 * \param[in] irq Interrupt number 
 *
 * \return uint32_t MQX_OK or error code
 */
_mqx_uint _nvic_int_enable
    (
        /* [IN] Interrupt number */
        _mqx_uint  irq
    )
{
    if (irq >= PSP_INT_FIRST_INTERNAL && irq <= PSP_INT_LAST_INTERNAL) {
        NVIC_EnableIRQ((IRQn_Type)irq);
    }
    else
        return MQX_INVALID_PARAMETER;

    return MQX_OK;
}

/*!
 * \brief Disable interrupt on cortex core NVIC
 * 
 * \param[in] irq Interrupt number
 *
 * \return uint32_t MQX_OK or error code
 */
_mqx_uint _nvic_int_disable
    (
        /* [IN] Interrupt number -*/
        _mqx_uint  irq
    )
{
    if (irq >= PSP_INT_FIRST_INTERNAL && irq <= PSP_INT_LAST_INTERNAL) {
        NVIC_DisableIRQ((IRQn_Type)irq);
    }
    else
        return MQX_INVALID_PARAMETER;

    return MQX_OK;
}
