#include "scheduler.h"

/*=============================================================
                      SCHEDULER VARIABLES
 ==============================================================*/

static _queue_id g_RequestQueue;
static _pool_id g_SchedulerRequestMessagePool;
static _pool_id g_TaskCreateMessagePool;
static _pool_id g_TaskDeleteMessagePool;

/*=============================================================
                      INITIALIZATION
 ==============================================================*/

void _initializeSchedulerMessagePools(uint32_t initialSize, uint32_t growthRate, uint32_t maxSize){

	// Initialize message pools
	g_SchedulerRequestMessagePool = _msgpool_create(sizeof(SchedulerRequestMessage), initialSize, growthRate, maxSize);
	g_TaskCreateMessagePool = _msgpool_create(sizeof(TaskCreateMessage), initialSize, growthRate, maxSize);
	g_TaskDeleteMessagePool = _msgpool_create(sizeof(TaskDeleteMessage), initialSize, growthRate, maxSize);

	// Check that initialization succeeded
	if(g_SchedulerRequestMessagePool == MSGPOOL_NULL_POOL_ID ||
			g_TaskCreateMessagePool == MSGPOOL_NULL_POOL_ID ||
			g_TaskDeleteMessagePool == MSGPOOL_NULL_POOL_ID){
		printf("Failed to create a scheduler message pool.\n");
		_task_block();
	}
}

static _queue_id _initializeQueue(int queueNum){
	_queue_id queueId = _msgq_open(queueNum, 0);
	if(queueId == 0){
		printf("Failed to open queue %d.\n", queueNum);
		_task_block();
	}
	return queueId;
}

/*=============================================================
                          MESSAGES
 ==============================================================*/

static TaskCreateMessagePtr _initializeTaskCreateMessage(uint32_t templateIndex, uint32_t deadline, _queue_id responseQueue){
	// Allocate message
	TaskCreateMessagePtr message = (TaskCreateMessagePtr)_msg_alloc(g_TaskCreateMessagePool);

	// Ensure allocation succeeded
	if (message == NULL) {
	 printf("Could not allocate a TaskCreateMessage.\n");
	 _task_block();
	}

	// Set message fields
	message->HEADER.TARGET_QID = g_RequestQueue;
	message->HEADER.SOURCE_QID = responseQueue;
	message->MessageType = CREATE;
	message->TemplateIndex = templateIndex;
	message->Deadline = deadline;

	return message;
}

static TaskDeleteMessagePtr _initalizeTaskDeleteMessage(_task_id taskId, _queue_id responseQueue){
	// Allocate message
	TaskDeleteMessagePtr message = (TaskDeleteMessagePtr)_msg_alloc(g_TaskDeleteMessagePool);

	// Ensure allocation succeeded
	if (message == NULL) {
	 printf("Could not allocate a TaskDeleteMessage.\n");
	 _task_block();
	}

	// Set message fields
	message->HEADER.TARGET_QID = g_RequestQueue;
	message->HEADER.SOURCE_QID = responseQueue;
	message->MessageType = DELETE;
	message->TaskId = taskId;

	return message;
}

static SchedulerRequestMessagePtr _initializeSchedulerRequestMessage(_queue_id responseQueue){
	// Allocate message
	SchedulerRequestMessagePtr message = (SchedulerRequestMessagePtr)_msg_alloc(g_SchedulerRequestMessagePool);

	// Ensure allocation succeeded
	if (message == NULL) {
	 printf("Could not allocate a SchedulerRequestMessage.\n");
	 _task_block();
	}

	message->HEADER.TARGET_QID = g_RequestQueue;
	message->HEADER.SOURCE_QID = responseQueue;

	return message;
}

static SchedulerRequestMessagePtr _initializeRequestActiveMessage(_queue_id responseQueue){
	SchedulerRequestMessagePtr message = _initializeSchedulerRequestMessage(responseQueue);
	message->MessageType = REQUEST_ACTIVE;
	return message;
}

static SchedulerRequestMessagePtr _initializeRequestOverdueMessage(_queue_id responseQueue){
	SchedulerRequestMessagePtr message = _initializeSchedulerRequestMessage(responseQueue);
	message->MessageType = REQUEST_OVERDUE;
	return message;
}

/*=============================================================
                      TASK LIST MANAGEMENT
 ==============================================================*/

/*=============================================================
                      INTERNAL INTERFACE
 ==============================================================*/

void _initializeScheduler(_queue_id requestQueue, uint32_t initialPoolSize, uint32_t poolGrowthRate, uint32_t maxPoolSize){
	g_RequestQueue = requestQueue;
	_initializeSchedulerMessagePools(initialPoolSize, poolGrowthRate, maxPoolSize);
}

void _handleSchedulerRequest(SchedulerRequestMessagePtr requestMessage){

}

void _handleDeadlineReached(){

}

uint32_t _getNextDeadline(){
	return NO_DEADLINE;
}

/*=============================================================
                      USER TASK INTERFACE
 ==============================================================*/

_task_id dd_tcreate(uint32_t templateIndex, uint32_t deadline){
	return 0;
}

bool dd_delete(_task_id task){
	return false;
}

bool dd_return_active_list(TaskList* taskList){
	return false;
}

bool dd_return_overdue_list(TaskList* taskList){
	return false;
}
