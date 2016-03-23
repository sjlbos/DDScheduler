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
_task_id _createPeriodicTask(uint32_t templateIndex, uint32_t deadline, uint32_t period);

// Helper functions
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
	  dd_tcreate(paramList->TemplateIndex, paramList->Deadline);
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

_task_id _createPeriodicTask(uint32_t templateIndex, uint32_t deadline, uint32_t period){
	//create struct to hold uint32_t templateIndex, uint32_t deadline, uint32_t period
	PeriodicTaskParameterListPtr paramList = _initializePeriodicTaskParameterList();
	paramList->TemplateIndex = templateIndex;
	paramList->Deadline = deadline;
	paramList->Period = period;
	TASK_TEMPLATE_STRUCT generatorTaskTemplate = { 0, runPeriodicGenerator, PERIODIC_TASK_STACK_SIZE, DEFAULT_TASK_PRIORITY, "Periodic Task", 0, (uint32_t) paramList, 0};
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
		return _createPeriodicTask(templateIndex, deadline, period);
	}
}

bool _handleDeleteCommand(char* commandString){
	char token[2] = " ";
	strtok(commandString,token);
	uint32_t taskId = atoi(strtok(NULL,token));
	if(strtok(NULL,token) != NULL){
			return false;
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
	free(taskList);
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
	free(taskList);
	return;
}


/*=============================================================
                       HELPER FUNCTIONS
 ==============================================================*/

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

