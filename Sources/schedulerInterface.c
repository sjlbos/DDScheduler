#include "scheduler.h"
#include "handler.h"

//creates a task
void _handleCreate(char* outputString){
	return;
}

//deletes a task
void _handleDelete(char* outputString){
	return;
}

//provides feedback on active tasks
void _handleActive(char* outputString){
	return;
}

//provides feedback on overdue tasks
void _handleOverdue(char* outputString){
	return;
}

//handles the different commands that the user can input
bool HandleCommand(char* outputString){
	//parse the input string
	switch(outputString[0]){
		case 'c'://task create
			_handleCreate(outputString);
			break;
		case 'd'://task delete
			_handleDelete(outputString);
			break;
		case 'a'://request active list
			_handleActive(outputString);
			break;
		case 'o'://request overdue/dead list
			_handleOverdue(outputString);
			break;
		default://not a valid command
			return false;
	}
	return true;
}
