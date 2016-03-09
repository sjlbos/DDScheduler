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

#include "nio.h"
#include "fsl_os_abstraction.h"

static int nio_dummy_open(void *dev_context, const char *dev_name, int flags, void **fp_context, int *error);
static int nio_dummy_read(void *dev_context, void *fp_context, void *buf, size_t nbytes, int *error);
static int nio_dummy_write(void *dev_context, void *fp_context, const void *buf, size_t nbytes, int *error);
static _nio_off_t nio_dummy_lseek(void *dev_context, void *fp_context, _nio_off_t offset, int whence, int *error);
static int nio_dummy_ioctl(void *dev_context, void *fp_context, int *error, unsigned long int request, va_list ap);
static int nio_dummy_close(void *dev_context, void *fp_context, int *error);
static int nio_dummy_init(void *init_data, void **dev_context, int *error);
static int nio_dummy_deinit(void *dev_context, int *error);

const NIO_DEV_FN_STRUCT nio_dummy_dev_fn = {
    .OPEN = nio_dummy_open,
    .READ = nio_dummy_read,
    .WRITE = nio_dummy_write,
    .LSEEK = nio_dummy_lseek,
    .IOCTL = nio_dummy_ioctl,
    .CLOSE = nio_dummy_close,
    .INIT = nio_dummy_init,
    .DEINIT = nio_dummy_deinit,
};

typedef struct {
    size_t total;
    size_t rtotal;
    size_t wtotal;
    semaphore_t lock;
} NIO_DUMMY_DEV_CONTEXT_STRUCT;

typedef struct {
    size_t rcnt;
    size_t wcnt;
} NIO_DUMMY_FP_CONTEXT_STRUCT;

static int nio_dummy_open(void *dev_context, const char *dev_name, int flags, void **fp_context, int *error) {
    *fp_context = OSA_MemAlloc(sizeof(NIO_DUMMY_FP_CONTEXT_STRUCT));
    return 0;
}

static int nio_dummy_read(void *dev_context, void *fp_context, void *buf, size_t nbytes, int *error) {
    NIO_DUMMY_DEV_CONTEXT_STRUCT *devc = (NIO_DUMMY_DEV_CONTEXT_STRUCT*)dev_context;
    NIO_DUMMY_FP_CONTEXT_STRUCT *fpc = (NIO_DUMMY_FP_CONTEXT_STRUCT*)fp_context;

    fpc->rcnt += nbytes;

    OSA_SemaWait(&devc->lock, OSA_WAIT_FOREVER);
    devc->total += nbytes;
    devc->rtotal += nbytes;
    OSA_SemaPost(&devc->lock);

    /* todo: replace with OSA yield. Not part of OSA yet */
    OSA_TimeDelay(1);

    return 0;
}

static int nio_dummy_write(void *dev_context, void *fp_context, const void *buf, size_t nbytes, int *error) {
    NIO_DUMMY_DEV_CONTEXT_STRUCT *devc = (NIO_DUMMY_DEV_CONTEXT_STRUCT*)dev_context;
    NIO_DUMMY_FP_CONTEXT_STRUCT *fpc = (NIO_DUMMY_FP_CONTEXT_STRUCT*)fp_context;

    fpc->wcnt += nbytes;
    OSA_SemaWait(&devc->lock, OSA_WAIT_FOREVER);
    devc->total += nbytes;
    devc->wtotal += nbytes;
    OSA_SemaPost(&devc->lock);

    /* todo: replace with OSA yield. Not part of OSA yet */
    OSA_TimeDelay(1);

    return 0;
}

static _nio_off_t nio_dummy_lseek(void *dev_context, void *fp_context, _nio_off_t offset, int whence, int *error) {
    return 0;
}

static int nio_dummy_ioctl(void *dev_context, void *fp_context, int *error, unsigned long int request, va_list ap) {
    return 0;
}

static int nio_dummy_close(void *dev_context, void *fp_context, int *error) {
    OSA_MemFree(fp_context);
    return 0;
}

static int nio_dummy_init(void *init_data, void **dev_context, int *error) {
    NIO_DUMMY_DEV_CONTEXT_STRUCT *context;

    context = (NIO_DUMMY_DEV_CONTEXT_STRUCT*)OSA_MemAlloc(sizeof(NIO_DUMMY_DEV_CONTEXT_STRUCT));
    OSA_SemaCreate(&context->lock, 1);
    *dev_context = (void*)context;

    return 0;
}

static int nio_dummy_deinit(void *dev_context, int *error) {
    OSA_SemaDestroy(&((NIO_DUMMY_DEV_CONTEXT_STRUCT*)dev_context)->lock);
    OSA_MemFree(dev_context);
    return 0;
}
