#ifndef SOURCES_SCHEDULER_TASKMANAGEMENT_H_
#define SOURCES_SCHEDULER_TASKMANAGEMENT_H_

#include <stdio.h>
#include <stdbool.h>
#include <mqx.h>

#include "scheduler.h"

/*=============================================================
                    TASK MANAGER INTERFACE
 ==============================================================*/

void initializeTaskManager(const TASK_TEMPLATE_STRUCT taskTemplates[], uint32_t taskTemplateCount);
_task_id createTask(uint32_t templateIndex, uint32_t msToDeadline);
_task_id setCurrentTaskAsOverdue();
bool deleteTask(_task_id taskId);
TaskList getCopyOfActiveTasks();
TaskList getCopyOfOverdueTasks();
bool getNextTaskDeadline(MQX_TICK_STRUCT_PTR deadline);

#endif
