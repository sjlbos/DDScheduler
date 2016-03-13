#include "scheduler.h"

/*=============================================================
                      INITIALIZATION
 ==============================================================*/

_queue_id _initializeQueue(int queueNum){
	_queue_id queueId = _msgq_open(queueNum, 0);
	if(queueId == 0){
		printf("Failed to open queue %d.\n", queueNum);
		_task_block();
	}
	return queueId;
}

/*=============================================================
                      TASK LIST MANAGEMENT
 ==============================================================*/



/*=============================================================
                      USER TASK INTERFACE
 ==============================================================*/

_task_id dd_tcreate(){
	return 0;
}

bool dd_delete(_task_id task){
	return false;
}

bool dd_return_active_list(TaskList* taskList){
	return false;
}

bool dd_return_overdue_list(TaskList* taskList){
	return false;
}
