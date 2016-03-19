#include "statusUpdate.h"
#include "schedulerInterface.h"
#include "scheduler.h"
#include "handler.h"
#include "monitor.h"

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

//prints cpu utilization
void _handleCPUUtilization(uint32_t period){
	uint32_t idleMilliseconds = IdleTime();
	float CPUUtilization = idleMilliseconds/period;
	printf("CPU Utilization is: %f\n", CPUUtilization);
	return;
}

void StatusUpdate(uint32_t period){
	printf("Status Update: Interval %u\n", period);
	_handleActive();
	_handleOverdue();
	_handleCPUUtilization(period);
	return;
}
