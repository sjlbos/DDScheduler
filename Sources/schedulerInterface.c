#include "schedulerInterface.h"
#include "Scheduler/scheduler.h"
#include "TerminalDriver/handler.h"

#define PERIODIC_TASK_STACK_SIZE 700

/*=============================================================
                      FUNCTION PROTOTYPES
 ==============================================================*/

// Command handlers
_task_id _handleCreateCommand(char* commandString);
bool _handleDeleteCommand(char* commandString);
void _handleGetActiveCommand();
void _handleGetOverdueCommand();

// Helper functions
void _prettyPrintTaskList();
_task_id _createPeriodicTask(uint32_t templateIndex, uint32_t deadline, uint32_t period);

const char token[2] = " ";

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
	strtok(commandString,token);
	uint32_t templateIndex = atoi(strtok(NULL,token));
	uint32_t deadline = atoi(strtok(NULL,token));
	uint32_t period = atoi(strtok(NULL,token));
	if(strtok(NULL,token) != NULL){
		return 0;
	}
	if(period != 0){
		return _createPeriodicTask(templateIndex, deadline, period);
	}else{
		return dd_tcreate(templateIndex, deadline);
	}
}

bool _handleDeleteCommand(char* commandString){
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
				taskList->task->Deadline,
				taskList->task->CreatedAt.TICKS[0]);
		taskList = taskList->nextNode;
	}while(taskList!=NULL);
	printf("\n");
	return;
}

_task_id _createPeriodicTask(uint32_t templateIndex, uint32_t deadline, uint32_t period){
//	TASK_TEMPLATE_STRUCT GeneratorTask[] = {
//			{ 0, runPeriodicGenerator, PERIODIC_TASK_STACK_SIZE, DEFAULT_TASK_PRIORITY, "Periodic Task", 0, 10, 0}
//	};
//	_task_id newTaskId = _task_create(0, 0, (uint32_t) GeneratorTask[0]);
}
