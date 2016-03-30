#include "schedulerInterface.h"
#include "Scheduler/scheduler.h"
#include "TerminalDriver/handler.h"
#include "mqx_ksdk.h"

/*=============================================================
                      CONSTANTS
 ==============================================================*/

#define PERIODIC_TASK_STACK_SIZE 700
#define PERIODIC_GENERATOR_TASK_PRIORITY 5

/*=============================================================
                      FUNCTION PROTOTYPES
 ==============================================================*/

// Command handlers
_task_id _handleCreateCommand(char* commandString);
bool _handleDeleteCommand(char* commandString);
void _handleGetActiveCommand();
void _handleGetOverdueCommand();

// Periodic task generation
void runPeriodicGenerator(os_task_param_t task_init_data);
_task_id _createPeriodicTask(uint32_t templateIndex, uint32_t deadline, uint32_t period,  PeriodicTaskMappingPtr periodicTaskMap);

PeriodicTaskMappingPtr g_mapGeneratorWithTask[50];
uint32_t g_mapIndex = 0;

// Helper functions
void _freeTaskList(TaskList taskList);
void _prettyPrintTaskList();

/*=============================================================
                      PERIODIC GENERATOR TASK
 ==============================================================*/

void runPeriodicGenerator(os_task_param_t parameterListPtr){
  printf("[Scheduler Interface] Periodic Task created\n");
  PeriodicTaskParameterListPtr paramList = (PeriodicTaskParameterListPtr) parameterListPtr;

#ifdef PEX_USE_RTOS
  while (1) {
#endif
	  printf("[Scheduler Interface] Periodic task being run\n");
	  _task_id taskNum = dd_tcreate(paramList->TemplateIndex, paramList->Deadline);
	  if(taskNum == 0) {
		  printf("[Scheduler Interface] Could not create periodic task\n");
		  _task_block();
	  }
	  paramList->periodicTaskMap->userTaskID = taskNum;
	  printf("[Scheduler Interface] Periodic task ID is: %u\n", taskNum);
	  _time_delay(paramList->Period);

#ifdef PEX_USE_RTOS
  }
#endif
  free(paramList);
}

//static TASK_TEMPLATE_STRUCT g_GeneratorTaskTemplate = { 0, runPeriodicGenerator, PERIODIC_TASK_STACK_SIZE, DEFAULT_TASK_PRIORITY, "Periodic Task", 0, 0, 0};

static PeriodicTaskParameterListPtr _initializePeriodicTaskParameterList(){
	PeriodicTaskParameterListPtr list;
	if(!(list = (PeriodicTaskParameterListPtr) malloc(sizeof(PeriodicTaskParameterList)))){
		printf("[Scheduler Interface] Unable to allocate memory for periodic task parameter list struct.\n");
		_task_block();
	}
	memset(list, 0, sizeof(PeriodicTaskParameterList));
	return list;
}

static PeriodicTaskMappingPtr _initializePeriodicTaskMapping(){
	PeriodicTaskMappingPtr list;
	if(!(list = (PeriodicTaskMappingPtr) malloc(sizeof(PeriodicTaskMapping)))){
		printf("[Scheduler Interface] Unable to allocate memory for periodic task mapping struct.\n");
		_task_block();
	}
	memset(list, 0, sizeof(PeriodicTaskMapping));
	return list;
}

_task_id _createPeriodicTask(uint32_t templateIndex, uint32_t deadline, uint32_t period, PeriodicTaskMappingPtr periodicTaskMap){
	//create struct to hold uint32_t templateIndex, uint32_t deadline, uint32_t period
	PeriodicTaskParameterListPtr paramList = _initializePeriodicTaskParameterList();
	paramList->TemplateIndex = templateIndex;
	paramList->Deadline = deadline;
	paramList->Period = period;
	paramList->periodicTaskMap = periodicTaskMap;
	TASK_TEMPLATE_STRUCT generatorTaskTemplate = { 0, runPeriodicGenerator, PERIODIC_TASK_STACK_SIZE, 1, "Periodic Task", 0, (uint32_t) paramList, 0};
	_task_id newTaskId = _task_create(0, 0, (uint32_t) &generatorTaskTemplate);
	return newTaskId;
}


/*=============================================================
                      PUBLIC INTERFACE
 ==============================================================*/

//handles the different commands that the user can input
bool si_handleCommand(char* commandString){
	printf("[Scheduler Interface] Received string: %s\n", commandString);
	switch(commandString[0]){
		case 'c':// Create a task
			return _handleCreateCommand(commandString) != MQX_NULL_TASK_ID;
		case 'd':// Delete a task
			return _handleDeleteCommand(commandString);
		case 'a':// Request active task list
			_handleGetActiveCommand();
			break;
		case 'o': // Request overdue task list
			_handleGetOverdueCommand();
			break;
		default:
			printf("[Scheduler Interface] Invalid command.\n");
			return false;
	}
	return true;
}

/*=============================================================
                       COMMAND HANDLERS
 ==============================================================*/

_task_id _handleCreateCommand(char* commandString){
	char token[2] = " ";
	strtok(commandString,token);
	uint32_t templateIndex = atoi(strtok(NULL,token));
	uint32_t deadline = atoi(strtok(NULL,token));
	uint32_t period = atoi(strtok(NULL,token));
	if(period == 0){
		return dd_tcreate(templateIndex, deadline);
	} else {
		PeriodicTaskMappingPtr periodicTaskMap = _initializePeriodicTaskMapping();
		_task_id newPeriodicGenerator = _createPeriodicTask(templateIndex, deadline, period, periodicTaskMap);
		periodicTaskMap->periodicGeneratorID = newPeriodicGenerator;
		g_mapGeneratorWithTask[g_mapIndex] = periodicTaskMap;
		if(g_mapIndex > 47){
			printf("[Scheduler Interface] Too many periodic tasks. Do not add more.\n");
		} else {
			g_mapIndex++;
		}
		return newPeriodicGenerator;
	}
}

bool _handleDeleteCommand(char* commandString){
	char token[2] = " ";
	strtok(commandString,token);
	uint32_t taskId = atoi(strtok(NULL,token));
	if(strtok(NULL,token) != NULL){
			return false;
	}
	int i = 0;
	for(i = 0; i < g_mapIndex + 1; i++){
		if(g_mapGeneratorWithTask[i]->userTaskID == taskId){
			_task_destroy(g_mapGeneratorWithTask[i]->periodicGeneratorID);
		}
	}
	return dd_delete(taskId);
}

void _handleGetActiveCommand(){
	TaskList taskList;
	dd_return_active_list(&taskList);
	if(taskList == NULL){
		printf("[Scheduler Interface] No Active Tasks\n");
		return;
	}
	printf("[Scheduler Interface] Active Tasks:\n");
	_prettyPrintTaskList(taskList);
	_freeTaskList(taskList);
	return;
}

//provides feedback on overdue tasks
void _handleGetOverdueCommand(){
	TaskList taskList;
	dd_return_overdue_list(&taskList);
	if(taskList == NULL){
		printf("[Scheduler Interface] No Overdue Tasks\n");
		return;
	}
	printf("[Scheduler Interface] Overdue Tasks:\n");
	_prettyPrintTaskList(taskList);
	_freeTaskList(taskList);
	return;
}


/*=============================================================
                       HELPER FUNCTIONS
 ==============================================================*/

void _freeTaskList(TaskList taskList){
	if(taskList == NULL){
		return;
	}

	TaskListNodePtr currentNode = taskList;
	TaskListNodePtr nextNode;
	do{
		nextNode = currentNode->nextNode;
		free(currentNode->task);
		free(currentNode);
		currentNode = nextNode;
	}
	while(currentNode != NULL);
}

//print out tasks in a nice format
void _prettyPrintTaskList(TaskList taskList){
	do{
		printf("\n{\n ID: %u\n Deadline:  %u\n Created At: %u\n}\n",
				taskList->task->TaskId,
				taskList->task->Deadline.TICKS[0],
				taskList->task->CreatedAt.TICKS[0]);
		taskList = taskList->nextNode;
	}while(taskList!=NULL);
	printf("\n");
	return;
}

