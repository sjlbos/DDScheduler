#include "schedulerInterface.h"
#include "Scheduler/scheduler.h"
#include "TerminalDriver/handler.h"

const char token[2] = " ";

//creates a task
_task_id _handleCreate(char* outputString){
	strtok(outputString,token);
	uint32_t templateIndex = atoi(strtok(NULL,token));
	uint32_t dealine = atoi(strtok(NULL,token));
	if(strtok(NULL,token) != NULL){
		return 0;
	}
	return dd_tcreate(templateIndex, dealine);
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
			printf("Task ID:      %u\n", taskList->task->TaskId);
			printf("Task Deadline:%u\n", taskList->task->Deadline);
			printf("Task Type:    %u\n", taskList->task->TaskType);
			printf("Task Created: %u\n\n", taskList->task->CreatedAt);
			taskList = taskList->nextNode;
	}while(taskList!=NULL);
	return;
}

//provides feedback on active tasks
void _handleActive(){
	TaskList taskList;
	dd_return_active_list(&taskList);
	if(taskList == NULL) return;
	printf("Active Tasks:\n");
	_prettyPrintTaskList(taskList);
	free(taskList);
	return;
}

//provides feedback on overdue tasks
void _handleOverdue(){
	TaskList taskList;
	dd_return_overdue_list(&taskList);
	if(taskList == NULL) return;
	printf("OverDue Tasks:\n");
	_prettyPrintTaskList(taskList);
	free(taskList);
	return;
}

//handles the different commands that the user can input
bool HandleCommand(char* outputString){
	//parse the input string
	printf("Received string: %s\n", outputString);
	switch(outputString[0]){
		case 'c'://task create
			if(_handleCreate(outputString) == 0) return false;
			break;
		case 'd'://task delete
			if(!_handleDelete(outputString)) return false;
			break;
		case 'a'://request active list
			_handleActive();
			break;
		case 'o'://request overdue/dead list
			_handleOverdue();
			break;
		default://not a valid command
			return false;
	}
	return true;
}
