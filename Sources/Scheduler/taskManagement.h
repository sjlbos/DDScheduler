#ifndef SOURCES_SCHEDULER_TASKMANAGEMENT_H_
#define SOURCES_SCHEDULER_TASKMANAGEMENT_H_

#include <stdio.h>
#include <stdbool.h>
#include <mqx.h>

#include "scheduler.h"

/*=============================================================
                    TASK MANAGER INTERFACE
 ==============================================================*/

void intializeTaskManager(const TASK_TEMPLATE_STRUCT taskTemplates[], uint32_t taskTemplateCount);
_task_id createTask(uint32_t templateIndex, uint32_t ticksTodeadline);
bool deleteTask(_task_id taskId);
TaskList getCopyOfActiveTasks();
TaskList getCopyOfOverdueTasks();

#endif
