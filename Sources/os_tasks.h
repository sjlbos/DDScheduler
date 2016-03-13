
#ifndef __os_tasks_H
#define __os_tasks_H
/* MODULE os_tasks */

#include "fsl_device_registers.h"
#include "clockMan1.h"
#include "pin_init.h"
#include "osa1.h"
#include "mqx_ksdk.h"
#include "uart1.h"
#include "fsl_mpu1.h"
#include "fsl_hwtimer1.h"
#include "MainTask.h"
#include "ddScheduler.h"
#include "myUART.h"
#include "serialHandler.h"

#ifdef __cplusplus
extern "C" {
#endif 

/*=============================================================
                          CONSTANTS
 ==============================================================*/

#define HANDLER_INTERRUPT_QUEUE_ID 8
#define HANDLER_INPUT_QUEUE_ID 9

#define SERIAL_MESSAGE_POOL_INITIAL_SIZE 16
#define SERIAL_MESSAGE_POOL_GROWTH_RATE 16
#define SERIAL_MESSAGE_POOL_MAX_SIZE 2048

#define INTERRUPT_MESSAGE_POOL_INITIAL_SIZE 1
#define INTERRUPT_MESSAGE_POOL_GROWTH_RATE 1
#define INTERRUPT_MESSAGE_POOL_MAX_SIZE 16

#define SCHEDULER_QUEUE_ID 10

#define SCHEDULER_MESSAGE_POOL_INITIAL_SIZE 16
#define SCHEDULER_MESSAGE_POOL_GROWTH_RATE 8
#define SCHEDULER_MESSAGE_POOL_MAX_SIZE 64


/*=============================================================
                     TASK ENTRY POINTS
 ==============================================================*/

void runScheduler(os_task_param_t task_init_data);
void runSerialHandler(os_task_param_t task_init_data);

/* END os_tasks */

#ifdef __cplusplus
}  /* extern "C" */
#endif 

#endif 
