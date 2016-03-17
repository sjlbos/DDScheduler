#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include <mqx.h>
#include <message.h>
#include <sem.h>

#ifndef SOURCES_SCHEDULER_H_
#define SOURCES_SCHEDULER_H_

/*=============================================================
                      	  CONSTANTS
 ==============================================================*/

#define NO_DEADLINE 0
#define MAX_CONCURRENT_REQUESTS 100
#define MIN_RESPONSE_QUEUE_ID 20

/*=============================================================
                      EXPORTED TYPES
 ==============================================================*/

typedef void (*TaskEntryPoint)();

typedef struct TaskDefinition{
	char* Name;
	TaskEntryPoint EntryPoint;
	uint32_t Period;
} TaskDefinition, * TaskDefinitionPtr;

typedef struct SchedulerTask{
	uint32_t TaskId;
	uint32_t Deadline;
	uint32_t TaskType;
	uint32_t CreatedAt;
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

typedef struct TaskCreateResponseMessage{
	MESSAGE_HEADER_STRUCT HEADER;
	_task_id TaskId;
} TaskCreateResponseMessage, * TaskCreateResponseMessagePtr;

typedef struct TaskDeleteResponseMessage{
	MESSAGE_HEADER_STRUCT HEADER;
	bool Result;
} TaskDeleteResponseMessage, * TaskDeleteResponseMessagePtr;

typedef struct TaskListResponseMessage{
	MESSAGE_HEADER_STRUCT HEADER;
	TaskList Tasks;
} TaskListResponseMessage, * TaskListResponseMessagePtr;

typedef union SchedulerMessage{
	SchedulerRequestMessage RequestMessage;
	TaskCreateMessage CreateMessage;
	TaskDeleteMessage DeleteMessage;
	TaskCreateResponseMessage CreateResponse;
	TaskDeleteResponseMessage DeleteResponse;
	TaskListResponseMessage TaskListResponse;
} SchedulerMessage, *SchedulerMessagePtr;

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
