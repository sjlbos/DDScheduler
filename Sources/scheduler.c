#include "scheduler.h"

/*=============================================================
                      SCHEDULER VARIABLES
 ==============================================================*/

static _queue_id g_RequestQueue;
static _pool_id g_SchedulerMessagePool;
static MUTEX_STRUCT g_QueueNumMutex;

static TaskDefinition* g_TaskDefinitions;
static uint32_t g_TaskDefinitionCount;
static TaskList g_ActiveTasks;
static TaskList g_OverdueTasks;

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
		printf("Could not allocate a TaskCreateMessage.\n");
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
	message->Deadline = deadline;

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

/*=============================================================
                      TASK MANAGEMENT
 ==============================================================*/

static SchedulerTaskPtr _initializeSchedulerTask(){
	SchedulerTaskPtr task;
	if(!(task = (SchedulerTaskPtr) malloc(sizeof(SchedulerTask)))){
		printf("Unable to allocate memory for scheduler task struct.");
		_task_block();
	}
	memset(task, 0, sizeof(SchedulerTask));
	return task;
}

static TaskListNodePtr _initializeTaskListNode(){
	TaskListNodePtr node;
	if(!(node = (TaskListNodePtr) malloc(sizeof(TaskListNode)))){
		printf("Unable to allocate memory for task list node struct.\n");
		_task_block();
	}
	memset(node, 0, sizeof(TaskListNode));
	return node;
}

static void _addTaskToSequentialList(SchedulerTaskPtr task, TaskList* list){

	// Initialize a new list node
	TaskListNodePtr node = _initializeTaskListNode();
	node->task = task;

	// If the list is empty, set this node as the first node
	if (*list == NULL){
		*list = node;
	}
	// Otherwise, add it to the end of the list
	else{
		TaskListNodePtr currentNode = *list;
		for(;;){
			if(currentNode->nextNode == NULL){
				currentNode->nextNode = node;
				node->prevNode = currentNode;
				break;
			}
			currentNode = currentNode->nextNode;
		}
	}
}

static void _addTaskToDeadlinePrioritizedList(SchedulerTaskPtr newTask, TaskList* list){
	// Initialize a new list node
	TaskListNodePtr node = _initializeTaskListNode();
	node->task = newTask;

	// If the list is empty, set this node as the first node
	if (*list == NULL){
		*list = node;
	}
	// Otherwise, add the node in a position such that the list remains sorted by task priority
	else{
		TaskListNodePtr currentNode = *list;
		for(;;){
			// If the new task preempts the current task, place the new task before the current task in the list
			if(newTask->Deadline < currentNode->task->Deadline){
				node->nextNode = currentNode;
				node->prevNode = currentNode->prevNode;
				if(currentNode->prevNode != NULL){
					currentNode->prevNode->nextNode = node;
				}
				currentNode->prevNode = node;
				break;
			}
			// Otherwise, if we have reached the end of list without preempting any tasks, add the new task to the end of the list
			else if (currentNode->nextNode == NULL){
				currentNode->nextNode = node;
				node->prevNode = currentNode;
				break;
			}
			currentNode = currentNode->nextNode;
		}
	}
}

static _task_id _createTask(uint32_t templateIndex, uint32_t deadline){

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

static void _handleRequestOverdueTasksMessage(SchedulerRequestMessagePtr message){

}

/*=============================================================
                       INTERNAL INTERFACE
 ==============================================================*/

void _initializeScheduler(_queue_id requestQueue, const TaskDefinition taskDefinitions[], uint32_t taskDefinitionCount){
	g_RequestQueue = requestQueue;
	g_TaskDefinitions = taskDefinitions;
	g_TaskDefinitionCount = taskDefinitionCount;

	_initializeSchedulerMessagePool();
	_initializeQueueNumMutex();
	g_ActiveTasks = NULL;
	g_OverdueTasks = NULL;
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

	return newTaskId;
}

bool dd_delete(_task_id task){

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

	return result;
}

bool dd_return_active_list(TaskList* taskList){

	// Initialize response queue and request message
	_queue_id responseQueue = _initializeQueue(_getResponseQueueId());
	SchedulerRequestMessagePtr requestActiveMessage = _initializeRequestActiveMessage(responseQueue);

	// Put create message on scheduler's request queue
	if(_msgq_send(requestActiveMessage) != TRUE){
		printf("Unable to send request active tasks message.\n");
		_task_block();
	}

	// Wait for response from scheduler
	TaskListResponseMessagePtr response = (TaskListResponseMessagePtr) _msgq_receive(responseQueue, 0);
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

	return true;
}

bool dd_return_overdue_list(TaskList* taskList){

	// Initialize response queue and request message
	_queue_id responseQueue = _initializeQueue(_getResponseQueueId());
	SchedulerRequestMessagePtr requestOverdueMessage = _initializeRequestOverdueMessage(responseQueue);

	// Put create message on scheduler's request queue
	if(_msgq_send(requestOverdueMessage) != TRUE){
		printf("Unable to send request active tasks message.\n");
		_task_block();
	}

	// Wait for response from scheduler
	TaskListResponseMessagePtr response = (TaskListResponseMessagePtr) _msgq_receive(responseQueue, 0);
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

	return true;
}
