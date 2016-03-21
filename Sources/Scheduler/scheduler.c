#include "scheduler.h"
#include "taskManagement.h"

/*=============================================================
                    LOCAL GLOBAL VARIABLES
 ==============================================================*/

static _queue_id g_RequestQueue;			// The queue on which request messages will be sent the the scheduler
static _pool_id g_SchedulerMessagePool;		// The scheduler's private message pool
static MUTEX_STRUCT g_QueueNumMutex;		// A mutex to ensure concurrent scheduler requests get assigned different response queue numbers

/*=============================================================
                      FUNCTION PROTOTYPES
 ==============================================================*/

// Request handlers
static void _handleCreateTaskMessage(TaskCreateMessagePtr message);
static void _handleDeleteTaskMessage(TaskDeleteMessagePtr message);
static void _handleRequestActiveTasksMessage(SchedulerRequestMessagePtr message);
static void _handleRequestOverdueTasksMessage(SchedulerRequestMessagePtr message);

// Scheduler initialization
static void _initializeSchedulerMessagePool();
static _queue_id _initializeQueue(int queueNum);
static void _initializeQueueNumMutex();

// Message initialization
static uint32_t _getResponseQueueId();
static SchedulerMessagePtr _initializeSchedulerMessage();
static TaskCreateMessagePtr _initializeTaskCreateMessage(uint32_t templateIndex, uint32_t deadline, _queue_id responseQueue);
static TaskDeleteMessagePtr _initializeTaskDeleteMessage(_task_id taskId, _queue_id responseQueue);
static SchedulerRequestMessagePtr _initializeSchedulerRequestMessage(_queue_id responseQueue);
static SchedulerRequestMessagePtr _initializeRequestActiveMessage(_queue_id responseQueue);
static SchedulerRequestMessagePtr _initializeRequestOverdueMessage(_queue_id responseQueue);
static TaskCreateResponseMessagePtr _initializeTaskCreateResponseMessage(_queue_id responseQueue, _task_id taskId);
static TaskDeleteResponseMessagePtr _initializeTaskDeleteResponseMessage(_queue_id responseQueue, bool result);
static TaskListResponseMessagePtr _initializeTaskListResponseMessage(_queue_id responseQueue, TaskList taskList);

/*=============================================================
                      USER TASK INTERFACE
 ==============================================================*/

_task_id dd_tcreate(uint32_t templateIndex, uint32_t deadline){

	// Initialize response queue and create message
	_queue_id responseQueue = _initializeQueue(_getResponseQueueId());
	TaskCreateMessagePtr createMessage = _initializeTaskCreateMessage(templateIndex, deadline, responseQueue);

	// Put create message on scheduler's request queue
	if(_msgq_send(createMessage) != TRUE){
		printf("[User] Unable to send create task message.\n");
		_task_block();
	}

	// Wait for response from scheduler
	TaskCreateResponseMessagePtr response = (TaskCreateResponseMessagePtr) _msgq_receive(responseQueue, 0);
	if(response == NULL){
		printf("[User] Failed to receive a create task response from the scheduler.\n");
		_task_block();
	}

	// Get the ID of the new task
	_task_id newTaskId = response->TaskId;

	// Free the response message and destroy the queue
	_msg_free(response);
	if(_msgq_close(responseQueue) != TRUE){
		printf("[User] Unable to close response queue.\n");
		_task_block();
	}

	return newTaskId;
}

bool dd_delete(_task_id taskId){
	TaskDeleteMessagePtr deleteMessage;
	_queue_id responseQueue;
	bool taskIsDeletingSelf = (_task_get_id() == taskId);

	if(taskIsDeletingSelf){
		deleteMessage = _initializeTaskDeleteMessage(taskId, MSGQ_NULL_QUEUE_ID);
	}
	else{
		// Initialize response queue and delete message
		responseQueue = _initializeQueue(_getResponseQueueId());
		deleteMessage = _initializeTaskDeleteMessage(taskId, responseQueue);
	}

	// Put delete message on scheduler's request queue
	if(_msgq_send(deleteMessage) != TRUE){
		printf("[User] Unable to send delete task message.\n");
		_task_block();
	}

	if(!taskIsDeletingSelf){
		// Wait for response from scheduler
		TaskDeleteResponseMessagePtr response = (TaskDeleteResponseMessagePtr) _msgq_receive(responseQueue, 0);
		if(response == NULL){
			printf("[User] Failed to receive a delete task response from the scheduler.\n");
			_task_block();
		}

		// Get the request result
		bool result = response->Result;

		// Free the response message and destroy the queue
		_msg_free(response);
		if(_msgq_close(responseQueue) != TRUE){
			printf("[User] Unable to close response queue.\n");
			_task_block();
		}
		return result;
	}

	_task_block();
	return false;
}

bool dd_return_active_list(TaskList* taskList){

	// Initialize response queue and request message
	_queue_id responseQueue = _initializeQueue(_getResponseQueueId());
	SchedulerRequestMessagePtr requestActiveMessage = _initializeRequestActiveMessage(responseQueue);

	// Put create message on scheduler's request queue
	if(_msgq_send(requestActiveMessage) != TRUE){
		printf("[User] Unable to send request active tasks message.\n");
		_task_block();
	}

	// Wait for response from scheduler
	TaskListResponseMessagePtr response = (TaskListResponseMessagePtr) _msgq_receive(responseQueue, 0);
	if(response == NULL){
		printf("[User] Failed to receive a task list response from the scheduler.\n");
		_task_block();
	}

	// Get the returned task list
	*taskList = response->Tasks;

	// Free the response message and destroy the queue
	_msg_free(response);
	if(_msgq_close(responseQueue) != TRUE){
		printf("[User] Unable to close response queue.\n");
		_task_block();
	}

	return true;
}

bool dd_return_overdue_list(TaskList* taskList){

	// Initialize response queue and request message
	_queue_id responseQueue = _initializeQueue(_getResponseQueueId());
	SchedulerRequestMessagePtr requestOverdueMessage = _initializeRequestOverdueMessage(responseQueue);

	// Put create message on scheduler's request queue
	if(_msgq_send(requestOverdueMessage) != TRUE){
		printf("[User] Unable to send request active tasks message.\n");
		_task_block();
	}

	// Wait for response from scheduler
	TaskListResponseMessagePtr response = (TaskListResponseMessagePtr) _msgq_receive(responseQueue, 0);
	if(response == NULL){
		printf("[User] Failed to receive a task list response from the scheduler.\n");
		_task_block();
	}

	// Get the returned task list
	*taskList = response->Tasks;

	// Free the response message and destroy the queue
	_msg_free(response);
	if(_msgq_close(responseQueue) != TRUE){
		printf("[User] Unable to close response queue.\n");
		_task_block();
	}

	return true;
}


/*=============================================================
                    SCHEDULER TASK INTERFACE
 ==============================================================*/

void _initializeScheduler(_queue_id requestQueue, const TASK_TEMPLATE_STRUCT taskTemplates[], uint32_t taskTemplateCount){
	g_RequestQueue = requestQueue;
	initializeTaskManager(taskTemplates, taskTemplateCount);
	_initializeSchedulerMessagePool();
	_initializeQueueNumMutex();
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
			printf("[Scheduler] Encountered an invalid request type.\n");
			_task_block();
	}
}

void _handleDeadlineReached(){
	setCurrentTaskAsOverdue();
}

bool _getNextDeadline(MQX_TICK_STRUCT_PTR deadline){
	return getNextTaskDeadline(deadline);
}


/*=============================================================
                       REQUEST HANDLERS
 ==============================================================*/

static void _handleCreateTaskMessage(TaskCreateMessagePtr message){
	printf("[Scheduler] Received a create request for a task at index %u with deadline %u.\n",
		message->TemplateIndex, message->TicksToDeadline);

	// Create a new task
	_task_id newTaskId = createTask(message->TemplateIndex, message->TicksToDeadline);

	// Allocate response message
	TaskCreateResponseMessagePtr response = _initializeTaskCreateResponseMessage(message->HEADER.SOURCE_QID, newTaskId);

	// Send response
	if(_msgq_send(response) != TRUE){
		printf("Unable to send create task response.\n");
		_task_block();
	}
}

static void _handleDeleteTaskMessage(TaskDeleteMessagePtr message){
	printf("[Scheduler] Received a delete request for task %u.\n", message->TaskId);

	// Delete the task
	bool result = deleteTask(message->TaskId);

	// If a task is deleting itself, its response queue will be NULL
	if(message->HEADER.SOURCE_QID == MSGQ_NULL_QUEUE_ID){
		return;
	}

	// Allocate response message
	TaskDeleteResponseMessagePtr response = _initializeTaskDeleteResponseMessage(message->HEADER.SOURCE_QID, result);

	// Send response
	if(_msgq_send(response) != TRUE){
		printf("Unable to send task delete response.\n");
		_task_block();
	}
}

static void _handleRequestActiveTasksMessage(SchedulerRequestMessagePtr message){
	printf("[Scheduler] Received a request for active tasks.\n");

	// Get active tasks
	TaskList activeTasks = getCopyOfActiveTasks();

	// Allocate response message
	TaskListResponseMessagePtr response = _initializeTaskListResponseMessage(message->HEADER.SOURCE_QID, activeTasks);

	// Send response
	if(_msgq_send(response) != TRUE){
		printf("Unable to send active tasks response.\n");
		_task_block();
	}
}

static void _handleRequestOverdueTasksMessage(SchedulerRequestMessagePtr message){
	printf("[Scheduler] Received a request for overdue tasks.\n");

	// Get overdue tasks
	TaskList overdueTasks = getCopyOfOverdueTasks();

	// Allocate response message
	TaskListResponseMessagePtr response = _initializeTaskListResponseMessage(message->HEADER.SOURCE_QID, overdueTasks);

	// Send response
	if(_msgq_send(response) != TRUE){
		printf("[Scheduler] Unable to send active tasks response.\n");
		_task_block();
	}
}


/*=============================================================
                      INITIALIZATION
 ==============================================================*/

static void _initializeSchedulerMessagePool(){

	// Initialize message pools
	g_SchedulerMessagePool = _msgpool_create(
			sizeof(SchedulerMessage),
			SCHEDULER_MESSAGE_POOL_INITIAL_SIZE,
			SCHEDULER_MESSAGE_POOL_GROWTH_RATE,
			SCHEDULER_MESSAGE_POOL_MAX_SIZE);

	// Check that initialization succeeded
	if(g_SchedulerMessagePool == MSGPOOL_NULL_POOL_ID){
		printf("Failed to create the scheduler message pool.\n");
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

static void _initializeQueueNumMutex(){
	MUTEX_ATTR_STRUCT mutexAttributes;
	if(_mutatr_init(&mutexAttributes) != MQX_OK){
		printf("Mutex attribute initialization failed.\n");
		_task_block();
	}

	if(_mutex_init(&g_QueueNumMutex, &mutexAttributes) != MQX_OK){
		printf("QueueNumMutex initialization failed.\n");
		_task_block();
	}
}

/*=============================================================
                          MESSAGES
 ==============================================================*/

static uint32_t _getResponseQueueId(){
	static uint32_t currentQueueNum = MIN_RESPONSE_QUEUE_ID;

	// Lock queue number mutex
	if(_mutex_lock(&g_QueueNumMutex) != MQX_OK){
		printf("Mutex lock failed.\n");
		_task_block();
	}

	// Increment queue number
	currentQueueNum = (currentQueueNum == MAX_RESPONSE_QUEUE_ID) ? MIN_RESPONSE_QUEUE_ID : currentQueueNum + 1;

	// Release mutex
	_mutex_unlock(&g_QueueNumMutex);

	return currentQueueNum;
}

static SchedulerMessagePtr _initializeSchedulerMessage(){

	SchedulerMessagePtr message = (SchedulerMessagePtr)_msg_alloc(g_SchedulerMessagePool);

	if (message == NULL){
		printf("Could not allocate a SchedulerMessage.\n");
		_task_block();
	}

	memset(message, 0, sizeof(SchedulerMessage));

	return message;
}

static TaskCreateMessagePtr _initializeTaskCreateMessage(uint32_t templateIndex, uint32_t deadline, _queue_id responseQueue){
	// Allocate message
	TaskCreateMessagePtr message = (TaskCreateMessagePtr) _initializeSchedulerMessage();

	// Set message fields
	message->HEADER.TARGET_QID = g_RequestQueue;
	message->HEADER.SOURCE_QID = responseQueue;
	message->MessageType = CREATE;
	message->TemplateIndex = templateIndex;
	message->TicksToDeadline = deadline;

	return message;
}

static TaskDeleteMessagePtr _initializeTaskDeleteMessage(_task_id taskId, _queue_id responseQueue){
	TaskDeleteMessagePtr message = (TaskDeleteMessagePtr) _initializeSchedulerMessage();
	message->HEADER.TARGET_QID = g_RequestQueue;
	message->HEADER.SOURCE_QID = responseQueue;
	message->MessageType = DELETE;
	message->TaskId = taskId;
	return message;
}

static SchedulerRequestMessagePtr _initializeSchedulerRequestMessage(_queue_id responseQueue){
	SchedulerRequestMessagePtr message = (SchedulerRequestMessagePtr) _initializeSchedulerMessage();
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

static TaskCreateResponseMessagePtr _initializeTaskCreateResponseMessage(_queue_id responseQueue, _task_id taskId){
	TaskCreateResponseMessagePtr message = (TaskCreateResponseMessagePtr) _initializeSchedulerMessage();
	message->HEADER.TARGET_QID = responseQueue;
	message->HEADER.SOURCE_QID = g_RequestQueue;
	message->TaskId = taskId;
	return message;
}

static TaskDeleteResponseMessagePtr _initializeTaskDeleteResponseMessage(_queue_id responseQueue, bool result){
	TaskDeleteResponseMessagePtr message = (TaskDeleteResponseMessagePtr) _initializeSchedulerMessage();
	message->HEADER.TARGET_QID = responseQueue;
	message->HEADER.SOURCE_QID = g_RequestQueue;
	message->Result = result;
	return message;
}

static TaskListResponseMessagePtr _initializeTaskListResponseMessage(_queue_id responseQueue, TaskList taskList){
	TaskListResponseMessagePtr message = (TaskListResponseMessagePtr) _initializeSchedulerMessage();
	message->HEADER.TARGET_QID = responseQueue;
	message->HEADER.SOURCE_QID = g_RequestQueue;
	message->Tasks = taskList;
	return message;
}


