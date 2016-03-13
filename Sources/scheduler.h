#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include <mqx.h>
#include <message.h>

#ifndef SOURCES_SCHEDULER_H_
#define SOURCES_SCHEDULER_H_

/*=============================================================
                      	  CONSTANTS
 ==============================================================*/

#define NO_DEADLINE 0

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

typedef TaskListNodePtr TaskList;

typedef enum MessageType{
	CREATE,
	DELETE,
	REQUEST_ACTIVE,
	REQUEST_OVERDUE
} MessageType;

typedef struct SchedulerRequestMessage{
	MESSAGE_HEADER_STRUCT HEADER;
	MessageType MessageType;
} SchedulerRequestMessage, * SchedulerRequestMessagePtr;

typedef struct TaskCreateMessage{
	MESSAGE_HEADER_STRUCT HEADER;
	MessageType MessageType;
	uint32_t TemplateIndex;
	uint32_t Deadline;
} TaskCreateMessage, * TaskCreateMessagePtr;

typedef struct TaskDeleteMessage{
	MESSAGE_HEADER_STRUCT HEADER;
	MessageType MessageType;
	_task_id TaskId;
} TaskDeleteMessage, * TaskDeleteMessagePtr;

typedef struct TaskListMessage{
	MESSAGE_HEADER_STRUCT HEADER;
	TaskList Tasks;
} TaskListMessage, * TaskListMessagePtr;

/*=============================================================
                      USER TASK INTERFACE
 ==============================================================*/

_task_id dd_tcreate(uint32_t templateIndex, uint32_t deadline);
bool dd_delete(_task_id task);
bool dd_return_active_list(TaskList* taskList);
bool dd_return_overdue_list(TaskList* taskList);

/*=============================================================
                      INTERNAL INTERFACE
 ==============================================================*/

void _initializeScheduler(_queue_id requestQueue, uint32_t initialPoolSize, uint32_t poolGrowthRate, uint32_t maxPoolSize);
void _handleSchedulerRequest(SchedulerRequestMessagePtr requestMessage);
void _handleDeadlineReached();
uint32_t _getNextDeadline();

#endif /* SOURCES_SCHEDULER_H_ */
