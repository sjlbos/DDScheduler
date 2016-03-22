#include "schedulerInterface.h"
#include "Scheduler/scheduler.h"
#include "TerminalDriver/handler.h"

#define PERIODIC_TASK_STACK_SIZE 700

const char token[2] = " ";

_task_id _create_periodic(uint32_t templateIndex, uint32_t deadline, uint32_t period){
//	TASK_TEMPLATE_STRUCT GeneratorTask[] = {
//			{ 0, runPeriodicGenerator, PERIODIC_TASK_STACK_SIZE, DEFAULT_TASK_PRIORITY, "Periodic Task", 0, 10, 0}
//	};
//	_task_id newTaskId = _task_create(0, 0, (uint32_t) GeneratorTask[0]);
}

//creates a task
_task_id _handleCreate(char* outputString){
	strtok(outputString,token);
	uint32_t templateIndex = atoi(strtok(NULL,token));
	uint32_t deadline = atoi(strtok(NULL,token));
	uint32_t period = atoi(strtok(NULL,token));
	if(strtok(NULL,token) != NULL){
		return 0;
	}
	if(period != 0){
		return _create_periodic(templateIndex, deadline, period);
	}else{
		return dd_tcreate(templateIndex, deadline);
	}
}

//deletes a task
bool _handleDelete(char* outputString){
	strtok(outputString,token);
	uint32_t taskId = atoi(strtok(NULL,token));
	if(strtok(NULL,token) != NULL){
			return false;
	}
	return dd_delete(taskId);
}

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

//provides feedback on active tasks
void _handleActive(){
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
void _handleOverdue(){
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

//handles the different commands that the user can input
bool HandleCommand(char* outputString){
	//parse the input string
	printf("[Scheduler Interface] Received string: %s\n", outputString);
	switch(outputString[0]){
		case 'c':// Create a task
			return _handleCreate(outputString) != MQX_NULL_TASK_ID;
		case 'd':// Delete a task
			return _handleDelete(outputString);
		case 'a':// Request active task list
			_handleActive();
			break;
		case 'o': // Request overdue task list
			_handleOverdue();
			break;
		default:
			printf("[Scheduler Interface] Invalid command.\n");
			return false;
	}
	return true;
}
