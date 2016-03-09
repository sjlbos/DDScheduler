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
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <assert.h>
#include "nio.h"
#include "nio_prv.h"
#include "ioctl.h"
#include "fsl_os_abstraction.h"
#if defined(FSL_RTOS_MQX)
#include "mqx_inc.h"
#endif

#if defined ( __IAR_SYSTEMS_ICC__ )
/* MISRA C 2004 rule 20.5 suppress: Error[Pm101]: The error indicator errno shall not be used */
/* MISRA C 2004 rule 14.2 suppress: Error[Pm049]: All non-null statements shall have a side-effect */
/* Pm049 occurs only on va_end() provided by stdarg.h header */
_Pragma ("diag_suppress= Pm101,Pm049")
#endif

/** Global pointer to File descriptor table
 */
NIO_FD_TBL_STRUCT *FD_TABLE;
/** Global Device list structure.
 */
NIO_DEV_LIST_STRUCT DEV_LIST;

//static int _nio_vioctl(NIO_DEV_STRUCT *dev, NIO_FILE *fp, int *error, unsigned long int request);
static int _nio_ioctl_wrap(NIO_IOCTL_FPTR ioctl_f, NIO_DEV_STRUCT *dev, NIO_FILE *fp, int *error, unsigned long int request, ...);

/** Lock FD table.
 * Protect file descriptors table for atomic and task safe operations.
 */
static int lock_fd_table(void) {
    assert(FD_TABLE != NULL);
    osa_status_t status = OSA_SemaWait(&FD_TABLE->LOCK, OSA_WAIT_FOREVER);
    return (kStatus_OSA_Success == status) ? 0 : -1;
}

/** Unlock FD table
 */
static int unlock_fd_table(void) {
    assert(FD_TABLE != NULL);
    osa_status_t status = OSA_SemaPost(&FD_TABLE->LOCK);
    return (kStatus_OSA_Success == status) ? 0 : -1;
}

/** Lock device list
 * Protect device list for atomic and task safe operations.
 */
static int lock_dev_list(void) {
    osa_status_t status = OSA_SemaWait(&DEV_LIST.LOCK, OSA_WAIT_FOREVER);
    return (kStatus_OSA_Success == status) ? 0 : -1;
}

/** Unlock device list
 */
static int unlock_dev_list(void) {
    osa_status_t status = OSA_SemaPost(&DEV_LIST.LOCK);
    return (kStatus_OSA_Success == status) ? 0 : -1;
}

/** Lock device
 * Protect device for atomic and task safe operations.
 */
static int lock_dev(NIO_DEV_STRUCT *dev) {
    osa_status_t status = OSA_SemaWait(&dev->LOCK, OSA_WAIT_FOREVER);
    return (kStatus_OSA_Success == status) ? 0 : -1;
}

/** Unlock device
 */
static int unlock_dev(NIO_DEV_STRUCT *dev) {
    osa_status_t status = OSA_SemaPost(&dev->LOCK);
    return (kStatus_OSA_Success == status) ? 0 : -1;
}

/** Lock file pointer
 * Protect file pointer for atomic and task safe operations.
 */
static int lock_fp(NIO_FILE *fp) {
    assert(fp != NULL);
    osa_status_t status = OSA_SemaWait(&fp->LOCK, OSA_WAIT_FOREVER);
    return (kStatus_OSA_Success == status) ? 0 : -1;
}

/** Unlock file pointer
 */
static int unlock_fp(NIO_FILE *fp) {
    assert(fp != NULL);
    osa_status_t status = OSA_SemaPost(&fp->LOCK);
    return (kStatus_OSA_Success == status) ? 0 : -1;
}

/** Get file pointer for selected file descriptor.
 */
static NIO_FILE* fd_get_fp(int fd) {
    NIO_FILE *fp = NULL;

    if ((fd >= 0) && (fd < FD_TABLE->MAX_OPEN)) {
        fp = FD_TABLE->FP[fd];
    }

    return fp;
}

/** Get new file descriptor
 * \return New file descriptor, or -1 if an error occurred - too many open files in system
 */
static int fd_new(void) {
    int i;

    for (i = 0; (i < FD_TABLE->MAX_OPEN) && (FD_TABLE->FP[i] != NULL); i++)
    {
    }

    return (i < FD_TABLE->MAX_OPEN) ? i : -1;
}

/** Free File Descriptor.
 * Must be called with locked FD_TABLE.
 */
static int fd_free(int fd) {

    FD_TABLE->FP[fd] = NULL;
    return 0;
}

/** Check device name.
 */
static int dev_check_name(const char *name) {
    unsigned int t;

    for (t = 0; t < NIO_DEV_NAME_LEN; t++) {
        if ((name[t] == 0) || (name[t] == ':')) {
            break;
        }
    }

    return (t == 0) || (t == NIO_DEV_NAME_LEN) ? -1 : 0;
}

/** Find device list item in system device list.
 * This function require locked DEV_LIST, but locking of DEV_LIST must
 * be implemented (called) outside.
 */
static NIO_DEV_LIST_ITM_STRUCT* dev_list_find(const char *name) {
    NIO_DEV_LIST_ITM_STRUCT *found = NULL;
    NIO_DEV_LIST_ITM_STRUCT *itm;
    int i;

    itm = DEV_LIST.ITEM;

    if (NULL == name) {
        return NULL;
    }

    while ((NULL == found) && (itm)) {

        for (i = 0; (itm->NAME[i] == name[i]) && (i < NIO_DEV_NAME_LEN) && (':' != name[i]) && ('\0' != name[i]); i++)
        {
        }

        if (((':' == name[i]) || ('\0' == name[i])) && ('\0' == itm->NAME[i])) {
            // device found
            found = itm;
            break;
        }

        itm = itm->NEXT;
    }

    return found;
}

/** Find device in system device list.
 * This function require locked DEV_LIST, but locking of DEV_LIST must
 * be implemented (called) outside.
 */
static NIO_DEV_STRUCT* dev_find(const char *name) {
    NIO_DEV_LIST_ITM_STRUCT *itm;
    itm = dev_list_find(name);

    return (itm) ? itm->DEV : NULL;
}

/** Add device to system device list.
 * This function require locked DEV_LIST, but locking of DEV_LIST must
 * be implemented (called) outside.
 * \param name < name of device
 * \param dev < pointer to device structure
 * \return successful completion return 0
 */
static int dev_add(const char *name, NIO_DEV_STRUCT *dev) {
    NIO_DEV_LIST_ITM_STRUCT *itm;
    int i;

    if ((name == NULL) || (*name == 0)) {
        return -1;
    }
    itm = OSA_MemAlloc(sizeof(NIO_DEV_LIST_ITM_STRUCT));

    if (itm) {
        // copy name to device list item
        for (i = 0; (i < NIO_DEV_NAME_LEN) && (name[i]); i++) {
            if ((':' == name[i]) || ('\0' == name[i])) {
                // device name end
                break;
            }

            itm->NAME[i] = name[i];
        }

        itm->NAME[i] = 0;

        itm->DEV = dev;
        //itm->fp = NULL;

        // add item to linked list
        itm->NEXT = DEV_LIST.ITEM;
        DEV_LIST.ITEM = itm;
    }

    return (itm) ? 0 : -1;
}

/** Remove device from system device list.
 * This function require locked DEV_LIST, but locking of DEV_LIST must
 * be implemented (called) outside.
 * \param name < name of device
 * \return successful completion return 0
 */
static int dev_rm(NIO_DEV_STRUCT *dev) {
    NIO_DEV_LIST_ITM_STRUCT *itm;
    NIO_DEV_LIST_ITM_STRUCT *prev = NULL;

    itm = DEV_LIST.ITEM;

    while ((itm) && (dev != itm->DEV)) {
        prev = itm;
        itm = itm->NEXT;
    }

    if (itm) {
        // remove itm from list
        if (prev) {
            prev->NEXT = itm->NEXT;
        }
        else {
            // item is a head
            DEV_LIST.ITEM = itm->NEXT;
        }

        OSA_MemFree(itm);         // free allocated item in device list
    }

    return (itm) ? 0 : -1;
}

/** add file pointer to doubly linked list
 */
static int dev_add_fp(NIO_FILE **head, NIO_FILE *fp) {
    fp->NEXT = *head;
    fp->PREV_NEXT = head;

    if (fp->NEXT)
    {
        fp->NEXT->PREV_NEXT = &fp->NEXT;    // *head->prev_next = &fp->next;
    }

    *head = fp;

    return 0;
}

/** remove file pointer to doubly linked list
 */
static int dev_rm_fp(NIO_FILE *fp) {
    if (fp->NEXT)       // last item in list
    {
        fp->NEXT->PREV_NEXT = fp->PREV_NEXT;
    }

    if (fp->PREV_NEXT)  // first item in list
    {
        *fp->PREV_NEXT = fp->NEXT;
    }

    return 0;
}

int _nio_init(int max_fd, int *error) {
    int err = 0;

    // allocate fd table
    FD_TABLE = (NIO_FD_TBL_STRUCT*)OSA_MemAlloc(max_fd * sizeof(NIO_FILE) + sizeof(NIO_FD_TBL_STRUCT));

    if (NULL != FD_TABLE) {
        #if defined(FSL_RTOS_MQX)
        KERNEL_DATA_STRUCT_PTR kernel_data;
        _GET_KERNEL_DATA(kernel_data);
        kernel_data->FD_TABLE = FD_TABLE;
        #endif
        FD_TABLE->MAX_OPEN = max_fd;

        // reset device list
        DEV_LIST.ITEM = NULL;
        if ( kStatus_OSA_Success != OSA_SemaCreate(&DEV_LIST.LOCK, 1)) {
            err = NIO_ENOMEM;
            #if defined(FSL_RTOS_MQX)
            KERNEL_DATA_STRUCT_PTR kernel_data;
            _GET_KERNEL_DATA(kernel_data);
            kernel_data->FD_TABLE = NULL;
            #endif
            OSA_MemFree(FD_TABLE);
        }

        // create semaphore for locking FD_TABLE
        else if ( kStatus_OSA_Success != OSA_SemaCreate(&FD_TABLE->LOCK, 1)) {
            err = NIO_ENOMEM;
            #if defined(FSL_RTOS_MQX)
            KERNEL_DATA_STRUCT_PTR kernel_data;
            _GET_KERNEL_DATA(kernel_data);
            kernel_data->FD_TABLE = NULL;
            #endif
            OSA_SemaDestroy(&DEV_LIST.LOCK);
            OSA_MemFree(FD_TABLE);
        }
        else {
          // set all fp to NULL
          memset(FD_TABLE->FP, 0, max_fd * sizeof(NIO_FILE));
        }
    }
    else {
        err = NIO_ENOMEM;
    }

    if (err && error) {
        *error = err;
    }

    return (err) ? -1 : 0;
}

NIO_DEV_STRUCT* _nio_dev_install(const char *name, const NIO_DEV_FN_STRUCT *fn, void *init_data, int *error) {
    NIO_DEV_STRUCT *dev = NULL;
    int res = 0;

    if (0 == dev_check_name(name)) {
        // find existing device by name
        dev = dev_find(name);
        if (NULL == dev) {
            // new, no exist, allocate structures
            dev = OSA_MemAlloc(sizeof(NIO_DEV_STRUCT));

            if (NULL != dev) {
                // init data (clean fd list, ...)
                dev->FN = fn;
                dev->FP = NULL;
                dev->USAGE = 0;
                dev->CONTEXT = NULL;

                if (kStatus_OSA_Success != OSA_SemaCreate(&dev->LOCK, 1)) {
                    if (error) { 
                        *error = NIO_ENOMEM;
                    }
                    OSA_MemFree(dev);
                    return NULL;
                }

                // call initialization fn
                if (dev->FN->INIT)
                {
                    res = dev->FN->INIT(init_data, &dev->CONTEXT, error);
                }

                if (0 == res) {
                    lock_dev_list();
                    res = dev_add(name, dev); // register device in system
                    unlock_dev_list();

                    if (res) {
                        // register device failed, we must deinit device and free resources
                        if (dev->FN->DEINIT)
                        {
                            res = dev->FN->DEINIT(&dev->CONTEXT, NULL);
                            assert(!res); //deinit failed, device driver problem
                            /* Avoid warning for error in release targets */
                            (void)res;
                        }

                        OSA_SemaDestroy(&dev->LOCK);
                        OSA_MemFree(dev);

                        dev = NULL;
                        if (error) {
                            *error = NIO_ENOMEM;
                        }
                    }
                }
            }
            else if (error) {
                *error = NIO_ENOMEM;
            }
        }
        else if (error) {
            *error = NIO_EEXIST;
        }
    }
    else if (error) {
        // bad device name
        *error = NIO_EINVAL;
    }

    return dev;
}

int _nio_dev_uninstall(const char *name, int *error) {
    NIO_DEV_STRUCT *dev;
    ///NIO_FILE *fp;
    int err = 0;

    // get device interface from name
    lock_dev_list();

    dev = dev_find(name);
    if (dev) {
        dev_rm(dev);    // remove device from system - from this point is not possible open new file on device - device does not exist in device list
        // lock_dev(dev);
        unlock_dev_list();

        // wait until close all files on device
        while (dev->USAGE) {
            // yield
            OSA_TaskYield();
        }

        // call deinitialization fn
        if (dev->FN->DEINIT)
        {
            int rv;
            rv = dev->FN->DEINIT(dev->CONTEXT, &err);
            if (0 <= rv) {
                err = 0;
            }
        }

        // release memory
        OSA_SemaDestroy(&dev->LOCK);
        OSA_MemFree(dev);
    }
    else {
        err = NIO_EEXIST;
        unlock_dev_list();
    }

    if (err && error) {
        *error = err;
    }

    return (err) ? -1 : 0;
}

int _nio_dev_uninstall_force(const char *name, int *error) {
    NIO_DEV_STRUCT *dev;
    NIO_FILE *fp;
    int err = 0; //accumlate some of the error
    int loop;

    // get device interface from name
    lock_dev_list();

    dev = dev_find(name);
    if (dev) {
        dev_rm(dev);    // remove device from system - from this point is not possible open new file on device - device does not exist in device list
        // lock_dev(dev);
        unlock_dev_list();

        // redirect all fp to error device - error device is simple NULL pointer
        lock_fd_table();
        lock_dev(dev);
        fp = dev->FP;
        while (NULL != fp) {
            lock_fp(fp);
            fp->DEV = NULL;
//            fp->USAGE++;
            unlock_fp(fp);

            fp = fp->NEXT;
        }
        unlock_dev(dev);
        unlock_fd_table();

        // wait until finish running operations
        while (dev->USAGE) {
            lock_fd_table();

            lock_dev(dev);  // protect device usage counter and unlink file from open file list in device
            fp = dev->FP;

            if ((NULL != fp) && (0 < fp->USAGE)) {
                unlock_dev(dev);

                // call abort for each file - break all actual running read, write, ioctl operations
                if (dev->FN->IOCTL) {
                    _nio_ioctl_wrap(dev->FN->IOCTL, dev->CONTEXT, fp->CONTEXT, error, IOCTL_ABORT);
                }
            }
            else
            {
                unlock_dev(dev);
            }

            loop = 1;
            do {
                lock_dev(dev);  // protect device usage counter and unlink file from open file list in device
                fp = dev->FP;
                if ((NULL == fp) || (0 == fp->USAGE)) {
                    loop = 0;
                }
                unlock_dev(dev);

                // yield
            } while (loop);

            unlock_fd_table();

            if (fp) {
                dev->FN->CLOSE(dev->CONTEXT, fp->CONTEXT, NULL);

                lock_fp(fp);
                fp->CONTEXT = NULL;     // close success every time! context was deallocated in low level close
                fp->USAGE--;
                unlock_fp(fp);

                // remove fp from device open fp list
                dev_rm_fp(fp);

                dev->USAGE--;
            }
            else {
                // yield
                OSA_TaskYield();
            }
        }

        // call deinitialization fn
        if (dev->FN->DEINIT)
        {
            err = dev->FN->DEINIT(dev->CONTEXT, error);
        }

        // release memory
        OSA_SemaDestroy(&dev->LOCK);
        OSA_MemFree(dev);
    }
    else {
        if (error) {
            err = *error = NIO_EEXIST;
        }
        unlock_dev_list();
    }

    return (err) ? -1 : 0;
}

static int _nio_open_intern(NIO_DEV_STRUCT *dev, const char *name, int flags, int mode, int *error) {
    void *fp_context;
    NIO_FILE *fp;
    int fd = -1;
    int err = 0;

    lock_fd_table();
    fd = fd_new();

    // check fd
    if (0 > fd) {
        // error - too many open files in system
        unlock_fd_table();
        if (error) {
            *error = NIO_ENFILE;
        }
        return fd;
    }
    else {
        // prepare filepointer
        fp = OSA_MemAlloc(sizeof(NIO_FILE));
        if (NULL != fp) {
            // initialize
            fp->DEV = NULL;
            fp->CONTEXT = NULL;
            fp->USAGE = 0;
            fp->NEXT = NULL;
            fp->PREV_NEXT = NULL;

            if( kStatus_OSA_Success != OSA_SemaCreate(&fp->LOCK, 1)) {
                if (error) {
                    *error = NIO_ENOMEM;
                }
                unlock_fd_table();
                OSA_MemFree(fp);
                return -1;
            }

            FD_TABLE->FP[fd] = fp;
        }
        else {
            // error - cannot allocate memory for file pointer
            unlock_fd_table();
            if (error) {
                *error = NIO_ENOMEM;
            }
            return -1;
        }
    }

    unlock_fd_table();

    if (NULL != dev) {
        if (NULL != dev->FN->OPEN) {
            // call real open (from low level device)
            err = dev->FN->OPEN(dev->CONTEXT, name, flags, &fp_context, error);
            if (err) {
                // error, exception
                // free resourcess
                lock_fd_table();
                FD_TABLE->FP[fd] = NULL;
                unlock_fd_table();

                OSA_SemaDestroy(&fp->LOCK);
                OSA_MemFree(fp);
                fp = NULL;
                fd = -1;
            }
            else {
                // success
                // prepare filepointer
                fp->DEV = dev;
                fp->CONTEXT = fp_context;

                // add fd to device open fp list
                // next operation must be protected by lock
                dev_add_fp(&dev->FP, fp);
            }
        }
        else {
            // error - operation not supported by device
            fd = -1;
            if (error) {
                *error = NIO_ENOTSUP;
            }
        }
    }
    else {
        // error - no such device
        fd = -1;
            if (error) {
                *error = NIO_EEXIST;
            }
    }

    return fd;
}

int _nio_open(const char *name, int flags, int *error) {
    NIO_DEV_STRUCT *dev;
    int fd = -1;
    int mode = 0;

    // get device interface from name
    lock_dev_list();

    dev = dev_find(name);
    if (dev) {
        lock_dev(dev);
        dev->USAGE++;       // device usage increment - start device using
        //unlock_dev(dev);

        unlock_dev_list();

        fd = _nio_open_intern(dev, name, flags, mode, error);
        if (0 > fd) {
            // open device failed, correct usege counter
            //lock_dev(dev);
            dev->USAGE--;
            //unlock_dev(dev);
        }
        unlock_dev(dev);
    }
    else {
        if (NULL != error) {
            *error = NIO_ENOENT;
        }
        unlock_dev_list();
    }

    return fd;
}

int _nio_close(int fd, int *error) {
    int err = -1;
    NIO_FILE *fp;
    NIO_DEV_STRUCT *dev;

    // lock fd table, table will be unlocked after safe fd release
    lock_fd_table();
    fp = fd_get_fp(fd);

    // check fp
    if (NULL == fp) {
        unlock_fd_table();
        if (NULL != error) {
            *error = NIO_EBADF;
        }
        else {
            err = 0;
        }
        return err;
    }

    // low level close is not implement, unsupported operation
    /*if (NULL != fp->dev && NULL == fp->dev->fn->close) {
        unlock_fd_table();
        status = NIO_EEXIST;
        goto ERROR;
    }*/

    lock_fp(fp);
    dev = fp->DEV;      // save device pointer for next closing operations
    fp->DEV = NULL;     // each new operation with fd will fail – error device
    fp->USAGE++;        // protect fd against device uninstall
    unlock_fp(fp);

    fd_free(fd);        // release file descriptor from FD_TABLE

    // no one can not do anything with file descriptor, because fd does not exist in table anymore

    unlock_fd_table();

    // wait until active io operation will be finished
    while (fp->USAGE > 1) {
        //yeld();
        OSA_TaskYield();
    }

    if (NULL != dev) {
        // device is real (not error device)
        // call real close if exist
        if (NULL != dev->FN->CLOSE) {
            err = dev->FN->CLOSE(dev->CONTEXT, fp->CONTEXT, error);
        }
        else {
            if (error) {
                *error = NIO_ENOTSUP;
            }
            err = -1;
        }

        lock_dev(dev);  // protect device usage counter and unlink file from open file list in device

        // decrement open fd on device counter
        dev->USAGE--;
        // remove fp from open fp list
        dev_rm_fp(fp);

        unlock_dev(dev);
    }


    // destroy fp lock and free FD_TABLE item (fp + ...)
    OSA_SemaDestroy(&fp->LOCK);
    OSA_MemFree(fp);

    return err;
}

ssize_t _nio_read(int fd, void *buf, size_t nbytes, int *error) {
    ssize_t res = -1;
    NIO_FILE *fp;
    NIO_DEV_STRUCT *dev;

    lock_fd_table();
    fp = fd_get_fp(fd);

    if (fp == NULL) {
        unlock_fd_table();
        if (error) {
            *error = NIO_EBADF;
        }
        return res;
    }

    if (fp->DEV == NULL) {
        unlock_fd_table();
        if (error) {
            *error = NIO_EIO;
        }
        return res;
    }

    lock_fp(fp);
    dev = fp->DEV;
    fp->USAGE++;
    unlock_fp(fp);

    unlock_fd_table();

    // call real read
    if (dev->FN->READ) {
        res = dev->FN->READ(dev->CONTEXT, fp->CONTEXT, buf, nbytes, error);
    }
    else if (error) {
        *error = NIO_ENOTSUP;
    }

    // decrement usage counter in fp
    lock_fp(fp);
    fp->USAGE--;
    unlock_fp(fp);

    return res;
}

ssize_t _nio_write(int fd, const void *buf, size_t nbytes, int *error) {
    ssize_t res = -1;
    NIO_FILE *fp;
    NIO_DEV_STRUCT *dev;

    lock_fd_table();
    fp = fd_get_fp(fd);

    if (fp == NULL) {
        unlock_fd_table();
        if (error) {
            *error = NIO_EBADF;
        }
        return res;
    }

    if (fp->DEV == NULL) {
        unlock_fd_table();
        if (error) {
            *error = NIO_EIO;
        }
        return res;
    }

    lock_fp(fp);
    dev = fp->DEV;
    fp->USAGE++;
    unlock_fp(fp);

    unlock_fd_table();

    // call real write
    if (dev->FN->WRITE) {
        res = dev->FN->WRITE(dev->CONTEXT, fp->CONTEXT, buf, nbytes, error);
    }
    else if (error) {
        *error = NIO_ENOTSUP;
    }

    // decrement usage counter in fp
    lock_fp(fp);
    fp->USAGE--;
    unlock_fp(fp);

    return res;
}

_nio_off_t _nio_lseek(int fd, _nio_off_t offset, int whence, int *error) {
    _nio_off_t res = -1;
    NIO_FILE *fp;
    NIO_DEV_STRUCT *dev;

    lock_fd_table();
    fp = fd_get_fp(fd);

    if (fp == NULL) {
        unlock_fd_table();
        if (error) {
            *error = NIO_EBADF;
        }
        return res;
    }

    if (fp->DEV == NULL) {
        unlock_fd_table();
        if (error) {
            *error = NIO_EBADF;
        }
        return res;
    }

    lock_fp(fp);
    dev = fp->DEV;
    fp->USAGE++;
    unlock_fp(fp);

    unlock_fd_table();

    // call low level lseek
    if (dev->FN->LSEEK) {
        res = dev->FN->LSEEK(dev->CONTEXT, fp->CONTEXT, offset, whence, error);
    }
    else if (error) {
        *error = NIO_ENOTSUP;
    }

    // decrement usage counter in fp
    lock_fp(fp);
    fp->USAGE--;
    unlock_fp(fp);

    return res;
}

static int _nio_ioctl_wrap(NIO_IOCTL_FPTR ioctl_f, NIO_DEV_STRUCT *dev, NIO_FILE *fp, int *error, unsigned long int request, ...) {
    int res = -1;
    va_list ap;
    va_start(ap, request);

    if (ioctl_f) {
        res = ioctl_f(dev, fp, error, request, ap);
    }
    else if (error) {
        *error = NIO_ENOTSUP;
    }

    va_end(ap);

    return res;
}

int _nio_vioctl(int fd, int *error, unsigned long int request, va_list ap)
{
    int res = -1;
    NIO_FILE *fp;
    NIO_DEV_STRUCT *dev;

    lock_fd_table();
    fp = fd_get_fp(fd);

    if (fp == NULL) {
        unlock_fd_table();
        if (error) {
            *error = NIO_EBADF;
        }
        return res;
    }

    if (fp->DEV == NULL) {
        unlock_fd_table();
        if (error) {
            *error = NIO_EBADF;
        }
        return res;
    }

    lock_fp(fp);
    dev = fp->DEV;
    fp->USAGE++;
    unlock_fp(fp);

    unlock_fd_table();

    // call low level ioctl
    if (dev->FN->IOCTL) {
        res = dev->FN->IOCTL(dev->CONTEXT, fp->CONTEXT, error, request, ap);
    }
    else if (error) {
        *error = NIO_ENOTSUP;
    }

    // decrement usage counter in fp
    lock_fp(fp);
    fp->USAGE--;
    unlock_fp(fp);

    return res;
}

int _nio_ioctl(int fd, int *error, unsigned long int request, ...) {
    int rv;
    va_list ap;

    va_start(ap, request);
    rv = _nio_vioctl(fd, error, request, ap);
    va_end(ap);

    return rv;
}

