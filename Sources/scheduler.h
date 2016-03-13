#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include <mqx.h>
#include <message.h>

#ifndef SOURCES_SCHEDULER_H_
#define SOURCES_SCHEDULER_H_

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

typedef enum MessageType{
	CREATE,
	DELETE,
	REQUEST_ACTIVE,
	REQUEST_OVERDUE
} MessageType;

typedef struct SchedulerRequestMessage{
	MESSAGE_HEADER_STRUCT HEADER;
	MessageType MessageType;
	_task_id TaskId;
} SchedulerRequestMessage, * SchedulerRequestMessagePtr;

typedef struct TaskListMessage{
	MESSAGE_HEADER_STRUCT HEADER;
	TaskList Tasks;
} TaskListMessage, * TaskListMessagePtr;

/*=============================================================
                      USER TASK INTERFACE
 ==============================================================*/

_task_id dd_tcreate();
bool dd_delete(_task_id task);
bool dd_return_active_list(TaskList* taskList);
bool dd_return_overdue_list(TaskList* taskList);

#endif /* SOURCES_SCHEDULER_H_ */
