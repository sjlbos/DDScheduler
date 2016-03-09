
/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
* Copyright 2004-2008 Embedded Access Inc.
* Copyright 1989-2008 ARC International
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
*   This file contains definitions private to the light weight
*   memory manger.
*
*
*END************************************************************************/
#ifndef __lwmem_prv_h__
#define __lwmem_prv_h__

/*--------------------------------------------------------------------------*/
/*
 *                    CONSTANT DEFINITIONS
 */

/* The correct value for the light weight memory pool VALID field */
#define LWMEM_POOL_VALID   (_mqx_uint)(0x6C6D6570)    /* "lmep" */

/* The smallest amount of memory that is allocated */
#define LWMEM_MIN_MEMORY_STORAGE_SIZE \
   ((_mem_size)(sizeof(LWMEM_BLOCK_STRUCT) + PSP_MEMORY_ALIGNMENT) & \
   PSP_MEMORY_ALIGNMENT_MASK)


/*--------------------------------------------------------------------------*/
/*
 *                      MACROS DEFINITIONS
 */

/*
 * get the location of the block pointer, given the address as provided
 * to the application by _lwmem_alloc.
 */
#define GET_LWMEMBLOCK_PTR(addr) \
   (LWMEM_BLOCK_STRUCT_PTR)((void *)((unsigned char *)(addr) - \
      sizeof(LWMEM_BLOCK_STRUCT)))

/*--------------------------------------------------------------------------*/
/*
 *                    DATATYPE DECLARATIONS
 */

/* LWMEM BLOCK STRUCT */
/*!
 * \cond DOXYGEN_PRIVATE
 *
 * \brief This structure is used to define the storage blocks used by the memory
 * manager in MQX.
 */
typedef struct lwmem_block_struct
{
   /*! \brief The size of the block. */
   _mem_size      BLOCKSIZE;

   /*! \brief The pool the block came from. */
   _lwmem_pool_id POOL;

   /*!
    * \brief For an allocated block, this is the task ID of the owning task.
    * When on the free list, this points to the next block on the free list.
    */
   union {
      void       *NEXTBLOCK;
      struct {
         _task_number    TASK_NUMBER;
         _mem_type       MEM_TYPE;
      } S;
   } U;

} LWMEM_BLOCK_STRUCT, * LWMEM_BLOCK_STRUCT_PTR;
/*! \endcond */

#define _GET_LWMEMBLOCK_TYPE(ptr)      (((LWMEM_BLOCK_STRUCT_PTR)GET_LWMEMBLOCK_PTR(ptr))->U.S.MEM_TYPE)

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
extern void *_lwmem_alloc_internal(
    _mem_size requested_size,
    TD_STRUCT_PTR td_ptr,
    _lwmem_pool_id pool_id,
    bool zero
);
/*! \endcond */

/*!
 * \cond DOXYGEN_PRIVATE
 */
extern _mqx_uint _lwmem_alloc_extend_internal(
    void *mem_ptr,
    _mem_size size
);
/*! \endcond */

/*!
 * \cond DOXYGEN_PRIVATE
 */
extern _mqx_uint _lwmem_free_part_internal(
    void *mem_ptr,
    _mem_size size
);
/*! \endcond */

/*!
 * \cond DOXYGEN_PRIVATE
 */
extern void *_lwmem_alloc_at_internal(
    _mem_size requested_size,
    void *requested_addr,
    TD_STRUCT_PTR  td_ptr,
    _lwmem_pool_id pool_id,
    bool zero
);
/*! \endcond */

/*!
 * \cond DOXYGEN_PRIVATE
 */
extern void *_lwmem_alloc_align_internal(
    _mem_size requested_size,
    _mem_size req_align,
    TD_STRUCT_PTR td_ptr,
    _lwmem_pool_id pool_id,
    bool zero
);
/*! \endcond */

/*!
 * \cond DOXYGEN_PRIVATE
 */
extern _mem_size _lwmem_get_free_internal(
    _lwmem_pool_id pool_id
);
/*! \endcond */

/*!
 * \cond DOXYGEN_PRIVATE
 */
extern void _lwmem_transfer_internal(
    void *memory_ptr,
    TD_STRUCT_PTR target_td
);
/*! \endcond */

/*!
 * \cond DOXYGEN_PRIVATE
 */
extern _mqx_uint _lwmem_transfer_td_internal(
    void *memory_ptr,
    TD_STRUCT_PTR source_td,
    TD_STRUCT_PTR target_td
);
/*! \endcond */

/*!
 * \cond DOXYGEN_PRIVATE
 */
extern _mqx_uint _lwmem_init_internal(void);
/*! \endcond */

/*!
 * \cond DOXYGEN_PRIVATE
 */
extern void *_lwmem_get_next_block_internal(
    TD_STRUCT_PTR td_ptr,
    void *in_block_ptr
);
/*! \endcond */

/*!
 * \cond DOXYGEN_PRIVATE
 */
extern _lwmem_pool_id _lwmem_create_pool_mapped(
    void *start,
    _mem_size size
);
/*! \endcond */

#if MQX_ENABLE_USER_MODE

/*!
 * \cond DOXYGEN_PRIVATE
 */
void *_usr_lwmem_alloc_internal(
    _mem_size requested_size
);
/*! \endcond */

#endif

#endif
#ifdef __cplusplus
}
#endif

#endif /* __lwmem_prv_h__ */
/* EOF */
