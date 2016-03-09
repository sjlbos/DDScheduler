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
* See license agreement file for full license terms including other restrictions.
*****************************************************************************
*
* Comments:
*
*   This file contains functions of the Two Level Segregate Fit, 
*   based on public domain licensed implementation from http://tlsf.baisoku.org/ .
*
*
*END************************************************************************/

#ifndef __tlsf_prv_h__
#define __tlsf_prv_h__
#include "tlsf.h"
/*--------------------------------------------------------------------------*/
/*
 *                    CONSTANT DEFINITIONS
 */


/*--------------------------------------------------------------------------*/
/*
 *                    DATATYPE DECLARATIONS
 */


/*--------------------------------------------------------------------------*/
/*
 *                      MACROS DEFINITIONS
 */

/*
 * get the location of the block pointer, given the address as provided
 * to the application by _tlsf_alloc.
 */
#define GET_TLSFBLOCK_PTR(addr)       tlsf_get_adaptation_from_ptr(addr)

#define _GET_TLSFBLOCK_TYPE(ptr)      _tlsf_get_type(ptr)

/*--------------------------------------------------------------------------*/
/*
 *                  PROTOTYPES OF FUNCTIONS
 */

#ifdef __cplusplus
extern "C" {
#endif
#ifndef __TAD_COMPILE__

/*!
 * \cond DOXYGEN_PRIVATE
 */
extern void* _tlsf_alloc_internal(
    _mem_size requested_size,
    TD_STRUCT_PTR td_ptr,
    tlsf_t pool_id,
    bool zero
);
/*! \endcond */

/*!
 * \cond DOXYGEN_PRIVATE
 */
extern void* _tlsf_alloc_align_internal(
    _mem_size requested_size,
    _mem_size req_align,
    TD_STRUCT_PTR td_ptr,
    tlsf_t pool_id,
    bool zero
);
/*! \endcond */

/*!
 * \cond DOXYGEN_PRIVATE
 */
extern void _tlsf_transfer_internal(
    void* memory_ptr,
    TD_STRUCT_PTR target_td
);
/*! \endcond */

/*!
 * \cond DOXYGEN_PRIVATE
 */
extern _mqx_uint _tlsf_transfer_td_internal(
    void* memory_ptr,
    TD_STRUCT_PTR source_td,
    TD_STRUCT_PTR target_td
);
/*! \endcond */

/*!
 * \cond DOXYGEN_PRIVATE
 */
extern _mqx_uint _tlsf_init_internal(void);
/*! \endcond */

/*!
 * \cond DOXYGEN_PRIVATE
 */
extern void* _tlsf_get_next_block_internal(
    TD_STRUCT_PTR td_ptr,
    void* memory_ptr
);
/*! \endcond */

/*!
 * \cond DOXYGEN_PRIVATE
 */
void* _tlsf_pool_create_limited_internal(
	void* tlsf_pool_ptr,
    unsigned char* start,
    unsigned char* end
);
/*! \endcond */


extern void* _tlsf_get_next_block_internal(
	TD_STRUCT_PTR td_ptr,
	void* memory_ptr
);
/*! \endcond */

#endif
#ifdef __cplusplus
}
#endif

#endif /* __tlsf_prv_h__ */
/* EOF */
