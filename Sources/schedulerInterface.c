#include "schedulerInterface.h"
#include "scheduler.h"
#include "handler.h"
#include "monitor.h"

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
