#include "taskManagement.h"

/*=============================================================
                     LOCAL GLOBAL VARIABLES
 ==============================================================*/

static const TASK_TEMPLATE_STRUCT* g_TaskTemplates;
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
static void _setCurrentlyRunningTask(SchedulerTaskPtr task);
static void _unblockTask(_task_id taskId);
static void _setTaskPriorityTo(uint32_t priority, _task_id taskId);

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
	g_CurrentTask = NULL;
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

	return newTaskId;
}

_task_id setCurrentTaskAsOverdue(){
	if(g_CurrentTask == NULL){
		return MQX_NULL_TASK_ID;
	}

	SchedulerTaskPtr overdueTask = g_CurrentTask;
	_setTaskPriorityTo(OVERDUE_TASK_PRIORITY, overdueTask->TaskId);
	_removeTaskWithIdFromTaskList(overdueTask->TaskId, &g_ActiveTasks);
	_addTaskToSequentialList(overdueTask, &g_OverdueTasks);

	return overdueTask->TaskId;
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
		// Set the currently running task to the next task in the active queue or NULL if there are no active tasks
		_setCurrentlyRunningTask((g_ActiveTasks == NULL) ? NULL : g_ActiveTasks->task);
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

bool getNextTaskDeadline(MQX_TICK_STRUCT_PTR deadline){
	if (g_CurrentTask == NULL){
		return false;
	}

	deadline->HW_TICKS = g_CurrentTask->Deadline.HW_TICKS;
	deadline->TICKS[0] = g_CurrentTask->Deadline.TICKS[0];
	deadline->TICKS[1] = g_CurrentTask->Deadline.TICKS[1];

	return true;
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
	return copy;
}

static void _scheduleNewTask(_task_id taskId, uint32_t ticksToDeadline){

	// Initialize task struct
	SchedulerTaskPtr newTask = _initializeSchedulerTask();
	newTask->TaskId = taskId;
	_time_get_ticks(&newTask->CreatedAt);
	newTask->Deadline = newTask->CreatedAt;
	newTask->Deadline.TICKS[0] += ticksToDeadline;

	// Add the new task to the list of active tasks
	uint32_t taskIndex = _addTaskToDeadlinePrioritizedList(newTask, &g_ActiveTasks);

	// If the new task has the highest priority, set it to running
	if(taskIndex == 0){
		_setCurrentlyRunningTask(newTask);
	}
	// Otherwise, set it to the default ready priority
	else{
		_setTaskPriorityTo(DEFAULT_TASK_PRIORITY, taskId);
	}
}

/*=============================================================
                    RUNNING TASK MANAGEMENT
 ==============================================================*/

static void _setCurrentlyRunningTask(SchedulerTaskPtr task){
	if (g_CurrentTask != NULL){
		if(task->TaskId == g_CurrentTask->TaskId){
			return;
		}
		_setTaskPriorityTo(DEFAULT_TASK_PRIORITY, g_CurrentTask->TaskId);
	}

	g_CurrentTask = task;
	if(g_CurrentTask != NULL){
		_setTaskPriorityTo(RUNNING_TASK_PRIORITY, g_CurrentTask->TaskId);
		_unblockTask(g_CurrentTask->TaskId);
	}
}

static void _unblockTask(_task_id taskId){
	TD_STRUCT_PTR taskDescriptor = _task_get_td(taskId);
	_task_ready(taskDescriptor);
}

static void _setTaskPriorityTo(uint32_t priority, _task_id taskId){
	uint32_t oldPriority;
	if(_task_set_priority(taskId, priority, &oldPriority) != MQX_OK){
		printf("Could not change priority of task %u.\n", taskId);
		_task_block();
	}
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
			if(newTask->Deadline.TICKS[0] < currentNode->task->Deadline.TICKS[0]){
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

