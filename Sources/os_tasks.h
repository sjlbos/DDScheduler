
#ifndef __os_tasks_H
#define __os_tasks_H
/* MODULE os_tasks */

#include <stdbool.h>
#include <stdio.h>
#include <mqx.h>

#include "fsl_device_registers.h"
#include "clockMan1.h"
#include "pin_init.h"
#include "osa1.h"
#include "mqx_ksdk.h"
#include "uart1.h"
#include "fsl_mpu1.h"
#include "fsl_hwtimer1.h"
#include "MainTask.h"
#include "rtos_main_task.h"
#include "ddScheduler.h"
#include "myUART.h"
#include "Cpu.h"

#include "Scheduler/scheduler.h"
#include "TerminalDriver/handler.h"
#include "schedulerInterface.h"
#include "monitor.h"
#include "serialHandler.h"
#include "statusUpdate.h"
#include "Events.h"

#ifdef __cplusplus
extern "C" {
#endif 

/*=============================================================
                          CONSTANTS
 ==============================================================*/

#define HANDLER_INTERRUPT_QUEUE_ID 8
#define HANDLER_INPUT_QUEUE_ID 9
#define SCHEDULER_QUEUE_ID 10
#define SCHEDULER_INTERFACE_QUEUE_ID 11

#define SERIAL_MESSAGE_POOL_INITIAL_SIZE 1
#define SERIAL_MESSAGE_POOL_GROWTH_RATE 1
#define SERIAL_MESSAGE_POOL_MAX_SIZE 16

#define INTERRUPT_MESSAGE_POOL_INITIAL_SIZE 1
#define INTERRUPT_MESSAGE_POOL_GROWTH_RATE 1
#define INTERRUPT_MESSAGE_POOL_MAX_SIZE 16


/*=============================================================
                     TASK ENTRY POINTS
 ==============================================================*/

void runScheduler(os_task_param_t task_init_data);
void runSerialHandler(os_task_param_t task_init_data);
void runSchedulerInterface(os_task_param_t task_init_data);
void runMonitor(os_task_param_t task_init_data);
void runStatusUpdate(os_task_param_t task_init_data);
void runUserTask(uint32_t numTicks);

/* END os_tasks */

/*=============================================================
                     INTERNAL FUNCTIONS
 ==============================================================*/


#ifdef __cplusplus
}  /* extern "C" */
#endif 

#endif 
