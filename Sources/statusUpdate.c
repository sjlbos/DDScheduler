#include "statusUpdate.h"
#include "schedulerInterface.h"
#include "Scheduler/scheduler.h"
#include "TerminalDriver/handler.h"

//print out tasks in a nice format
void _prettyPrintTaskList(TaskList taskList){
	do{
			printf("[Status Update] Task ID:      %u\n", taskList->task->TaskId);
			printf("[Status Update] Task Deadline:%u\n", taskList->task->Deadline);
			printf("[Status Update] Task Type:    %u\n", taskList->task->TaskType);
			printf("[Status Update] Task Created: %u\n\n", taskList->task->CreatedAt);
			taskList = taskList->nextNode;
	}while(taskList!=NULL);
	return;
}

//provides feedback on active tasks
void _handleActive(){
	TaskList taskList;
	dd_return_active_list(&taskList);
	if(taskList == NULL) return;
	printf("[Status Update] Active Tasks:\n");
	_prettyPrintTaskList(taskList);
	free(taskList);
	return;
}

//provides feedback on overdue tasks
void _handleOverdue(){
	TaskList taskList;
	dd_return_overdue_list(&taskList);
	if(taskList == NULL) return;
	printf("[Status Update] OverDue Tasks:\n");
	_prettyPrintTaskList(taskList);
	free(taskList);
	return;
}

//prints cpu utilization
void _handleCPUUtilization(uint32_t period, uint32_t timePassed){
	float CPUUtilization = timePassed/period;
	printf("[Status Update] CPU Utilization is: %0.4f\n", CPUUtilization);
	return;
}

//provide an update
void StatusUpdate(uint32_t *timePassed){
	uint32_t period = 10000;
	printf("[Status Update] Status Update: Interval %u\n", period);
	_handleCPUUtilization(period, *timePassed);
	*timePassed = 0;
	return;
}
