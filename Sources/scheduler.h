#include <ctype.h>
#include <stdbool.h>
#include <mqx.h>

#ifndef SOURCES_SCHEDULER_H_
#define SOURCES_SCHEDULER_H_

/*=============================================================
                         CONSTANTS
 ==============================================================*/

#define SCHEDULER_QUEUE_ID 10

#define SCHEDULER_MESSAGE_POOL_INITIAL_SIZE 16
#define SCHEDULER_MESSAGE_POOL_GROWTH_RATE 8
#define SCHEDULER_MESSAGE_POOL_MAX_SIZE 64

/*=============================================================
                      EXPORTED TYPES
 ==============================================================*/

typedef struct SchedulerTask{
	uint32_t taskId;
	uint32_t deadline;
	uint32_t taskType;
	uint32_t createdAt;
} SchedulerTask, *SchedulerTaskPtr;

typedef struct TaskListNode{
	SchedulerTaskPtr task;
	struct TaskListNode* nextNode;
	struct TaskListNode* prevNode;
} TaskListNode, *TaskListNodePtr;

typedef TaskListNodePtr* TaskList;


/*=============================================================
                      USER TASK INTERFACE
 ==============================================================*/

_task_id dd_tcreate();
uint32_t dd_delete(_task_id task);
uint32_t dd_return_active_list();
uint32_t dd_return_overdue_list();

#endif /* SOURCES_SCHEDULER_H_ */
