#include "scheduler.h"

/*=============================================================
                      SCHEDULER VARIABLES
 ==============================================================*/

static _queue_id g_RequestQueue;
static _pool_id g_SchedulerMessagePool;
static MUTEX_STRUCT g_QueueNumMutex;

static TASK_TEMPLATE_STRUCT* g_TaskTemplates;
static uint32_t g_TaskTemplateCount;
static SchedulerTaskPtr g_CurrentTask;
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

static uint32_t _addTaskToDeadlinePrioritizedList(SchedulerTaskPtr newTask, TaskList* list){
	uint32_t newTaskIndex = 0;

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
			newTaskIndex++;
			currentNode = currentNode->nextNode;
		}
	}
	return newTaskIndex;
}

static SchedulerTaskPtr _removeTaskWithIdFromTaskList(_task_id taskId, TaskList* list){

	// If the list is empty, return null
	if(*list == NULL){
		return NULL;
	}

	TaskListNodePtr currentNode = *list;
	for(;;){
		// If the current task has the matching Id, remove it from the list and return it
		if (currentNode->task->TaskId == taskId){
			if(currentNode->prevNode != NULL){
				currentNode->prevNode->nextNode = currentNode->nextNode;
			}
			if(currentNode->nextNode != NULL){
				currentNode->nextNode->prevNode = currentNode->prevNode;
			}
			SchedulerTaskPtr removedTask = currentNode->task;
			free(currentNode);
			return removedTask;
		}

		// Otherwise, return null if this is the end of the list
		if(currentNode->nextNode == NULL){
			return NULL;
		}

		// Otherwise, keep searching for the task
		currentNode = currentNode->nextNode;
	}
}

static void _setTaskAsReady(SchedulerTaskPtr task){
	uint32_t oldPriority;
	if(_task_set_priority(task->TaskId, DEFAULT_TASK_PRIORITY, &oldPriority) != MQX_OK){
		printf("Could not change priority of task %u.\n", task->TaskId);
		_task_block();
	}
}

static void _setTaskAsRunning(SchedulerTaskPtr task){
	// If the task is already running, do nothing
	if(task->TaskId == g_CurrentTask->TaskId){
		return;
	}
	uint32_t oldPriority;

	// Set the currently running task's priority to the default priority
	_setTaskAsReady(g_CurrentTask);

	// Set this task's priority to the running priority
	if(_task_set_priority(task->TaskId, RUNNING_TASK_PRIORITY, &oldPriority) != MQX_OK){
		printf("Could not change priority of task %u.\n", task->TaskId);
		_task_block();
	}
}

static void _setTaskAsOverdue(SchedulerTaskPtr task){

}

static void _addTaskToScheduler(SchedulerTaskPtr task){
	// Add the new task to the list of active tasks
	uint32_t taskIndex = _addTaskToDeadlinePrioritizedList(task, &g_ActiveTasks);

	if(taskIndex == 0){
		_setTaskAsRunning(task);
	}
	else{
		_setTaskAsReady(task);
	}
}

static bool _removeTaskFromScheduler(_task_id taskId){

	// Try to remove the task from the list of active tasks
	SchedulerTaskPtr removedTask = _removeTaskWithIdFromTaskList(taskId, &g_ActiveTasks);

	// If the task was not an active task, try deleting from the list of overdue tasks
	if(removedTask == NULL){
		removedTask = _removeTaskWithIdFromTaskList(taskId, &g_OverdueTasks);

		// If the task is still not found, it does not exist in the scheduler
		if (removedTask == NULL){
			return false;
		}
	}

	// If the deleted task is the currently running task...
	if(g_CurrentTask->TaskId == taskId){
		// If an ready task is available, set it as the currently running task
		if(g_ActiveTasks != NULL){
			_setTaskAsRunning(g_ActiveTasks->task);
		}
		// Otherwise, clear the currently running task
		else{
			g_CurrentTask = NULL;
		}
	}

	// Destroy deleted task and free its memory
	_task_destroy(taskId);
	free(removedTask);

	return true;
}

static _task_id _createTask(uint32_t templateIndex, uint32_t ticksTodeadline){
	// Ensure template index is valid
	if(templateIndex >= g_TaskTemplateCount){
		return MQX_NULL_TASK_ID;
	}

	// Create a blocked task
	_task_id newTaskId = _task_create_blocked(0, 0, (uint32_t) &g_TaskTemplates[templateIndex]);
	if (newTaskId == MQX_NULL_TASK_ID){
		printf("Unable to create task.\n");
		_task_block();
	}

	// Initialize task struct
	SchedulerTaskPtr newTask = _initializeSchedulerTask();
	newTask->TaskId = newTaskId;

	_addTaskToScheduler(newTask);

	// Unblock task
	TD_STRUCT_PTR taskDescriptor = _task_get_td(newTaskId);
	_task_ready(taskDescriptor);

	return newTaskId;
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

	// Create a new task
	_task_id newTaskId = _createTask(message->TemplateIndex, message->TicksToDeadline);

	// Allocate response message
	TaskCreateResponseMessagePtr response = _initializeTaskCreateResponseMessage(message->HEADER.SOURCE_QID, newTaskId);

	// Send response
	if(_msgq_send(response) != TRUE){
		printf("Unable to send create task response.\n");
		_task_block();
	}
}

static void _handleDeleteTaskMessage(TaskDeleteMessagePtr message){

	// Delete the task
	bool result = _removeTaskFromScheduler(message->TaskId);

	// Allocate response message
	TaskDeleteResponseMessagePtr response = _initializeTaskDeleteResponseMessage(message->HEADER.SOURCE_QID, result);

	// Send response
	if(_msgq_send(response) != TRUE){
		printf("Unable to send task delete response.\n");
		_task_block();
	}
}

static void _handleRequestActiveTasksMessage(SchedulerRequestMessagePtr message){

	// Get active tasks
	TaskList activeTasks = _getCopyOfActiveTasks();

	// Allocate response message
	TaskListResponseMessagePtr response = _initializeTaskListResponseMessage(message->HEADER.SOURCE_QID, activeTasks);

	// Send response
	if(_msgq_send(response) != TRUE){
		printf("Unable to send active tasks response.\n");
		_task_block();
	}
}

static void _handleRequestOverdueTasksMessage(SchedulerRequestMessagePtr message){

	// Get overdue tasks
	TaskList overdueTasks = _getCopyOfOverdueTasks();

	// Allocate response message
	TaskListResponseMessagePtr response = _initializeTaskListResponseMessage(message->HEADER.SOURCE_QID, overdueTasks);

	// Send response
	if(_msgq_send(response) != TRUE){
		printf("Unable to send active tasks response.\n");
		_task_block();
	}
}

/*=============================================================
                       INTERNAL INTERFACE
 ==============================================================*/

void _initializeScheduler(_queue_id requestQueue, const TASK_TEMPLATE_STRUCT taskTemplates[], uint32_t taskTemplateCount){
	g_RequestQueue = requestQueue;
	g_TaskTemplates = taskTemplates;
	g_TaskTemplateCount = taskTemplateCount;

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
