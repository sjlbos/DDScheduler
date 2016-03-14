#include "scheduler.h"

/*=============================================================
                      SCHEDULER VARIABLES
 ==============================================================*/

static _queue_id g_RequestQueue;
static _pool_id g_SchedulerRequestMessagePool;
static _pool_id g_TaskCreateMessagePool;
static _pool_id g_TaskDeleteMessagePool;
static void* g_RequestSemaphore;

/*=============================================================
                      INITIALIZATION
 ==============================================================*/

static void _initializeSchedulerMessagePools(uint32_t initialSize, uint32_t growthRate, uint32_t maxSize){

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

static void _initializeRequestSemaphore(){
	char* requestSemaphoreName = "RequestSem";

	if(_sem_create(requestSemaphoreName, MAX_CONCURRENT_REQUESTS, 0) != MQX_OK){
		printf("Unable to create semaphore.\n");
		_task_block();
	}
	if(_sem_open(requestSemaphoreName, &g_RequestSemaphore) != MQX_OK){
		printf("Unable to open semaphore \"%s\".", requestSemaphoreName);
		_task_block();
	}
}

/*=============================================================
                          MESSAGES
 ==============================================================*/

static uint32_t _getResponseQueueId(){
	return MIN_RESPONSE_QUEUE_ID + _sem_get_value(g_RequestSemaphore);
}

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

static TaskDeleteMessagePtr _initializeTaskDeleteMessage(_task_id taskId, _queue_id responseQueue){
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
                      TASK MANAGEMENT
 ==============================================================*/

static _task_id _createTask(uint32_t templateIndex, uin32_t deadline){
	return 0;
}

static bool _deleteTask(_task_id taskId){
	return false;
}

static TaskList _getCopyOfActiveTasks(){
	return NULL;
}

static TaskList _getCopyOfOverdueTasks(){
	return NULL;
}

/*=============================================================
                           HANDLERS
 ==============================================================*/

static void _handleCreateTaskMessage(TaskCreateMessagePtr message){

}

static void _handleDeleteTaskMessage(TaskDeleteMessagePtr message){

}

static void _handleRequestActiveTasksMessage(SchedulerRequestMessagePtr message){

}

static void _handleRequestOverdueTasksMessage(SchedulerRequestOverduePtr message){

}

/*=============================================================
                       INTERNAL INTERFACE
 ==============================================================*/

void _initializeScheduler(_queue_id requestQueue, uint32_t initialPoolSize, uint32_t poolGrowthRate, uint32_t maxPoolSize){
	g_RequestQueue = requestQueue;
	_initializeSchedulerMessagePools(initialPoolSize, poolGrowthRate, maxPoolSize);
	_initializeRequestSemaphore();
}

void _handleSchedulerRequest(SchedulerRequestMessagePtr requestMessage){
	switch(requestMessage->MessageType){
		case CREATE:
			_handleCreateTaskMessage((TaskCreateMessagePtr) requestMessage);
			break;
		case DELETE:
			_handleDeleteTaskMessage((TaskDeleteMessagePtr) requestMessage);
			break;
		case REQUEST_ACTIVE:
			_handleRequestActiveTasksMessage(requestMessage);
			break;
		case REQUEST_OVERDUE:
			_handleRequestOverdueTasksMessage(requestMessage);
			break;
		default:
			printf("Encountered an invalid request type.\n");
			_task_block();
	}
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
	// Wait on the request semaphore
	if(_sem_wait(g_RequestSemaphore, 0) != MQX_OK){
		printf("Encountered an error trying to wait on semaphore.\n");
		_task_block();
	}

	// Initialize response queue and create message
	_queue_id responseQueue = _initializeQueue(_getResponseQueueId());
	TaskCreateMessagePtr createMessage = _initializeTaskCreateMessage(templateIndex, deadline, responseQueue);

	// Put create message on scheduler's request queue
	if(_msgq_send(createMessage) != TRUE){
		printf("Unable to send create task message.\n");
		_task_block();
	}

	// Wait for response from scheduler
	TaskCreateResponseMessagePtr response = (TaskCreateResponseMessagePtr) _msgq_receive(responseQueue, 0);
	if(response == NULL){
		printf("Failed to receive a create task response from the scheduler.\n");
		_task_block();
	}

	// Get the ID of the new task
	_task_id newTaskId = response->TaskId;

	// Free the response message and destroy the queue
	_msg_free(response);
	if(_msgq_close(responseQueue) != TRUE){
		printf("Unable to close response queue.\n");
		_task_block();
	}

	// Release the request semaphore
	if(_sem_post(g_RequestSemaphore) != MQX_OK){
		printf("Encountered an error trying to release semaphore.\n");
		_task_block();
	}

	return newTaskId;
}

bool dd_delete(_task_id task){
	// Wait on the request semaphore
	if(_sem_wait(g_RequestSemaphore, 0) != MQX_OK){
		printf("Encountered an error trying to wait on semaphore.\n");
		_task_block();
	}

	// Initialize response queue and delete message
	_queue_id responseQueue = _initializeQueue(_getResponseQueueId());
	TaskDeleteMessagePtr deleteMessage = _initializeTaskDeleteMessage(task, responseQueue);

	// Put delete message on scheduler's request queue
	if(_msgq_send(deleteMessage) != TRUE){
		printf("Unable to send delete task message.\n");
		_task_block();
	}

	// Wait for response from scheduler
	TaskDeleteResponseMessagePtr response = (TaskDeleteResponseMessagePtr) _msgq_receive(responseQueue, 0);
	if(response == NULL){
		printf("Failed to receive a delete task response from the scheduler.\n");
		_task_block();
	}

	// Get the request result
	bool result = response->Result;

	// Free the response message and destroy the queue
	_msg_free(response);
	if(_msgq_close(responseQueue) != TRUE){
		printf("Unable to close response queue.\n");
		_task_block();
	}

	// Release the request semaphore
	if(_sem_post(g_RequestSemaphore) != MQX_OK){
		printf("Encountered an error trying to release semaphore.\n");
		_task_block();
	}

	return result;
}

bool dd_return_active_list(TaskList* taskList){
	// Wait on the request semaphore
	if(_sem_wait(g_RequestSemaphore, 0) != MQX_OK){
		printf("Encountered an error trying to wait on semaphore.\n");
		_task_block();
	}

	// Initialize response queue and request message
	_queue_id responseQueue = _initializeQueue(_getResponseQueueId());
	SchedulerRequestMessagePtr requestActiveMessage = _initializeRequestActiveMessage(responseQueue);

	// Put create message on scheduler's request queue
	if(_msgq_send(requestActiveMessage) != TRUE){
		printf("Unable to send request active tasks message.\n");
		_task_block();
	}

	// Wait for response from scheduler
	TaskListMessagePtr response = (TaskListMessagePtr) _msgq_receive(responseQueue, 0);
	if(response == NULL){
		printf("Failed to receive a task list response from the scheduler.\n");
		_task_block();
	}

	// Get the returned task list
	*taskList = response->Tasks;

	// Free the response message and destroy the queue
	_msg_free(response);
	if(_msgq_close(responseQueue) != TRUE){
		printf("Unable to close response queue.\n");
		_task_block();
	}

	// Release the request semaphore
	if(_sem_post(g_RequestSemaphore) != MQX_OK){
		printf("Encountered an error trying to release semaphore.\n");
		_task_block();
	}

	return true;
}

bool dd_return_overdue_list(TaskList* taskList){
	// Wait on the request semaphore
	if(_sem_wait(g_RequestSemaphore, 0) != MQX_OK){
		printf("Encountered an error trying to wait on semaphore.\n");
		_task_block();
	}

	// Initialize response queue and request message
	_queue_id responseQueue = _initializeQueue(_getResponseQueueId());
	SchedulerRequestMessagePtr requestOverdueMessage = _initializeRequestOverdueMessage(responseQueue);

	// Put create message on scheduler's request queue
	if(_msgq_send(requestOverdueMessage) != TRUE){
		printf("Unable to send request active tasks message.\n");
		_task_block();
	}

	// Wait for response from scheduler
	TaskListMessagePtr response = (TaskListMessagePtr) _msgq_receive(responseQueue, 0);
	if(response == NULL){
		printf("Failed to receive a task list response from the scheduler.\n");
		_task_block();
	}

	// Get the returned task list
	*taskList = response->Tasks;

	// Free the response message and destroy the queue
	_msg_free(response);
	if(_msgq_close(responseQueue) != TRUE){
		printf("Unable to close response queue.\n");
		_task_block();
	}

	// Release the request semaphore
	if(_sem_post(g_RequestSemaphore) != MQX_OK){
		printf("Encountered an error trying to release semaphore.\n");
		_task_block();
	}

	return true;
}
