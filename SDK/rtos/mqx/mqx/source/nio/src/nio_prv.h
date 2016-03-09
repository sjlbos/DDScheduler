#ifndef __nio_prv_h__
#define __nio_prv_h__
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

#include <stdint.h>

#include "nio.h"
#include "fsl_os_abstraction.h"


/** NIO File struct
 * Internal structure which represent opened file.
 */
typedef struct nio_file {
    NIO_DEV_STRUCT *DEV;            ///< pointer to opened device
    void *CONTEXT;                  ///< pointer to device data context per fp(allocated by low level driver in open)
    uint32_t USAGE;                 ///< usage counter, counting actual  running io operation with this fp (read, write, ioctl)
    semaphore_t LOCK;                ///< file pointer lock – atomic operation

    struct nio_file *NEXT;            ///< pointer to next fp in device fp linked list
    struct nio_file **PREV_NEXT;      ///< pointer to next pointer in previous fp in device fp linked list
                                      ///< pointer to pointer to this fp in the linked list (back reference)
} NIO_FILE;

/** File Descriptors table
 * Internal structure which represent file descriptors table - list of opened file descriptors in system.
 */
typedef struct nio_fd_tbl_struct {
    size_t MAX_OPEN;    ///< maximum open files
    semaphore_t LOCK;  ///< file descriptor table lock
    NIO_FILE *FP[];     ///< file pointer list (must be last item in struct)
} NIO_FD_TBL_STRUCT;

/** File pointer list item.
 * Item in dynamic linked list of file pointers. This list contain pointers to file pointers opened on device.
 * This is internal structure.
 */
/*
struct _fp_list_itm_t {
    NIO_FILE *fp;                   ///< file pointer
    struct _fp_list_t *next;    ///< pointer to next item in list
} fp_list_itm_t;
*/

/** Device list item.
 * This is internal structure.
 */
typedef struct nio_dev_list_itm_struct {
    char NAME[NIO_DEV_NAME_LEN];        ///< device name
    NIO_DEV_STRUCT *DEV;                     ///< pointer to installed device structure
//    NIO_FILE *fp_head;                      ///< head of opened fp list for device
    struct nio_dev_list_itm_struct *NEXT;       ///< next item
} NIO_DEV_LIST_ITM_STRUCT;

/** Device list.
 * Dynamic linked list for installed devices in system.
 * This is internal structure.
 */
typedef struct nio_dev_list_struct {
    NIO_DEV_LIST_ITM_STRUCT *ITEM;        ///< first item in list of installed devices
    semaphore_t LOCK;          ///< device list lock
} NIO_DEV_LIST_STRUCT;

#endif // __nio_prv_h__
