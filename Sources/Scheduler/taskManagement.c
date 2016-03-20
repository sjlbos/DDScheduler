#include "taskManagement.h"

/*=============================================================
                     LOCAL GLOBAL VARIABLES
 ==============================================================*/

static TASK_TEMPLATE_STRUCT* g_TaskTemplates;
static uint32_t g_TaskTemplateCount;
static SchedulerTaskPtr g_CurrentTask;
static TaskList g_ActiveTasks;
static TaskList g_OverdueTasks;

/*=============================================================
                      FUNCTION PROTOTYPES
 ==============================================================*/

// Task Creation
static SchedulerTaskPtr _initializeSchedulerTask();
static SchedulerTaskPtr _copySchedulerTask(SchedulerTaskPtr original);
static void _scheduleNewTask(_task_id taskId, uint32_t ticksToDeadline);

// Task Priority
static void _setTaskAsReady(SchedulerTaskPtr task);
static void _setTaskAsRunning(SchedulerTaskPtr task);
static void _setTaskAsOverdue(SchedulerTaskPtr task);

// Task List Management
static TaskListNodePtr _initializeTaskListNode();
static TaskList _copyTaskList(TaskList original);
static void _addTaskToSequentialList(SchedulerTaskPtr task, TaskList* list);
static uint32_t _addTaskToDeadlinePrioritizedList(SchedulerTaskPtr newTask, TaskList* list);
static SchedulerTaskPtr _removeTaskWithIdFromTaskList(_task_id taskId, TaskList* list);

/*=============================================================
                      PUBLIC INTERFACE
 ==============================================================*/

void initializeTaskManager(const TASK_TEMPLATE_STRUCT taskTemplates[], uint32_t taskTemplateCount){
	g_TaskTemplates = taskTemplates;
	g_TaskTemplateCount = taskTemplateCount;
	g_ActiveTasks = NULL;
	g_OverdueTasks = NULL;
}

_task_id createTask(uint32_t templateIndex, uint32_t ticksToDeadline){
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

	_scheduleNewTask(newTaskId, ticksToDeadline);

	// Unblock task
	TD_STRUCT_PTR taskDescriptor = _task_get_td(newTaskId);
	_task_ready(taskDescriptor);

	return newTaskId;
}

bool deleteTask(_task_id taskId){

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

TaskList getCopyOfActiveTasks(){
	return _copyTaskList(g_ActiveTasks);
}

TaskList getCopyOfOverdueTasks(){
	return _copyTaskList(g_OverdueTasks);
}

/*=============================================================
                         TASK CREATION
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

static SchedulerTaskPtr _copySchedulerTask(SchedulerTaskPtr original){
	SchedulerTaskPtr copy = _initializeSchedulerTask();
	copy->CreatedAt = original->CreatedAt;
	copy->Deadline = original->Deadline;
	copy->TaskId = original->TaskId;
	copy->TaskType = original->TaskType;
}

static void _scheduleNewTask(_task_id taskId, uint32_t ticksToDeadline){

	// Initialize task struct
	SchedulerTaskPtr newTask = _initializeSchedulerTask();
	newTask->TaskId = taskId;

	// Add the new task to the list of active tasks
	uint32_t taskIndex = _addTaskToDeadlinePrioritizedList(newTask, &g_ActiveTasks);

	// If the new task has the highest priority, set it to running
	if(taskIndex == 0){
		_setTaskAsRunning(newTask);
	}
	// Otherwise, set it to the default ready priority
	else{
		_setTaskAsReady(newTask);
	}
}

/*=============================================================
                    TASK PRIORITY MANAGEMENT
 ==============================================================*/

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

/*=============================================================
                      TASK LIST MANAGEMENT
 ==============================================================*/

static TaskListNodePtr _initializeTaskListNode(){
	TaskListNodePtr node;
	if(!(node = (TaskListNodePtr) malloc(sizeof(TaskListNode)))){
		printf("Unable to allocate memory for task list node struct.\n");
		_task_block();
	}
	memset(node, 0, sizeof(TaskListNode));
	return node;
}

static TaskList _copyTaskList(TaskList original){
	if (original == NULL){
		return NULL;
	}

	TaskListNodePtr originalNode = original;
	TaskListNodePtr copyNode = _initializeTaskListNode();
	TaskList copy = copyNode;
	for(;;){
		copyNode->task = _copySchedulerTask(originalNode->task);
		if(originalNode->nextNode == NULL){
			break;
		}
		originalNode = originalNode->nextNode;
		copyNode->nextNode = _initializeTaskListNode();
		copyNode = copyNode->nextNode;
	}
	return copy;
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

