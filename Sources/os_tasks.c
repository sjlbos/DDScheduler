#include "Cpu.h"
#include "Events.h"
#include "rtos_main_task.h"
#include "os_tasks.h"
#include <timer.h>

#ifdef __cplusplus
extern "C" {
#endif 

/*=============================================================
                        GLOBAL VARIABLES
 ==============================================================*/
uint32_t g_ticks;//ticks in the monitor
uint32_t g_milliseconds; //ms in the monitor
_pool_id g_InterruptMessagePool;	// A message pool for messages sent between from the UART event handler to the handler task
_pool_id g_SerialMessagePool;		// A message pool for messages sent between the handler task and its user tasks
HandlerPtr g_Handler;				// The global handler instance
MUTEX_STRUCT g_HandlerMutex;		// The mutex controlling access to the handler's internal state

/*=============================================================
                     USER TASK DEFINITIONS
 ==============================================================*/

#define USER_TASK_STACK_SIZE 700

const uint32_t USER_TASK_COUNT = 2;
const TASK_TEMPLATE_STRUCT USER_TASKS[] = {
		{ 0, runUserTask, USER_TASK_STACK_SIZE, DEFAULT_TASK_PRIORITY, "Periodic Task", 0, 10, 0},
		{ 0, runUserTask, USER_TASK_STACK_SIZE, DEFAULT_TASK_PRIORITY, "Run Once Task", 0, 5000, 0}
};

/*=============================================================
                       HELPER FUNCTIONS
 ==============================================================*/

_queue_id _initializeQueue(int queueNum){
	_queue_id queueId = _msgq_open(queueNum, 0);
	if(queueId == MSGQ_NULL_QUEUE_ID){
		printf("Failed to open queue %d.\n", queueNum);
		_task_block();
	}
	return queueId;
}

/*=============================================================
                       SCHEDULER TASK
 ==============================================================*/

void runScheduler(os_task_param_t task_init_data)
{
	printf("\r\nScheduler task started.\r\n");

	_queue_id requestQueue = _initializeQueue(SCHEDULER_INTERFACE_QUEUE_ID);
	_initializeScheduler(requestQueue, USER_TASKS, USER_TASK_COUNT);

	MQX_TICK_STRUCT nextDeadline;
	SchedulerRequestMessagePtr requestMessage;

#ifdef PEX_USE_RTOS
  while (1) {
#endif
	  requestMessage = NULL;
	  bool deadlineExists = _getNextDeadline(&nextDeadline);

	  // If the scheduler currently has tasks, wait for a new message or the next deadline
	  if(deadlineExists){
		  requestMessage = _msgq_receive_until(requestQueue, &nextDeadline);

		  // Handle reached deadlines
		  if(requestMessage == NULL){
			  _handleDeadlineReached();
			  continue;
		  }
	  }
	  // Otherwise wait indefinitely for a scheduler request
	  else{
		  requestMessage = _msgq_receive(requestQueue, 0);
	  }

	  // Handle scheduler requests
	  _handleSchedulerRequest(requestMessage);
	  _msg_free(requestMessage);

#ifdef PEX_USE_RTOS   
  }
#endif    
}

/*=============================================================
                       SERIAL HANDLER TASK
 ==============================================================*/

void _initializeHandlerMessagePools(){
	// Initialize interrupt message pool
	g_InterruptMessagePool = _msgpool_create(sizeof(InterruptMessage),
			INTERRUPT_MESSAGE_POOL_INITIAL_SIZE,
			INTERRUPT_MESSAGE_POOL_GROWTH_RATE,
			INTERRUPT_MESSAGE_POOL_MAX_SIZE);
	if(g_InterruptMessagePool == MSGPOOL_NULL_POOL_ID){
		printf("Failed to create the interrupt message pool.\n");
		_task_block();
	}

	// Initialize serial message pool
	g_SerialMessagePool = _msgpool_create(sizeof(SerialMessage),
			SERIAL_MESSAGE_POOL_INITIAL_SIZE,
			SERIAL_MESSAGE_POOL_GROWTH_RATE,
			SERIAL_MESSAGE_POOL_MAX_SIZE);
	if(g_SerialMessagePool == MSGPOOL_NULL_POOL_ID){
		printf("Failed to create the serial message pool.\n");
		_task_block();
	}
}

void runSerialHandler(os_task_param_t task_init_data)
{
	printf("\r\nHandler task started.\r\n");

		// Initialize queues and message pools
		_queue_id interruptQueue = _initializeQueue(HANDLER_INTERRUPT_QUEUE_ID);
		_queue_id inputQueue = _initializeQueue(HANDLER_INPUT_QUEUE_ID);
		_initializeHandlerMessagePools();

		// Initialize Handler
		Handler handler;
		g_Handler = &handler;
		_initializeHandlerMutex(&g_HandlerMutex);
		_initializeHandler(g_Handler, interruptQueue, inputQueue, myUART_IDX);

	#ifdef PEX_USE_RTOS
	  while (1) {
	#endif

	    // Wait for any incoming messages
		GenericMessagePtr receivedMessage = (GenericMessagePtr) _msgq_receive(MSGQ_ANY_QUEUE, 0);
		if (receivedMessage == NULL){
			   printf("Handler failed to receive a message.\n");
			   _task_block();
		}

		// Lock access to the handler while it processses the message
		if(_mutex_lock(&g_HandlerMutex) != MQX_OK){
			printf("Mutex lock failed.\n");
			_task_block();
		}

		// If the message is an serial message from the current writer, handle the writer input
		if(receivedMessage->HEADER.TARGET_QID == inputQueue){
			_handleWriteMessage((SerialMessagePtr) receivedMessage, g_Handler);
		}
		// If the message is an interrupt message from the UART event handler, handle the character press
		else if (receivedMessage->HEADER.TARGET_QID == interruptQueue){
			_handleInterruptMessage((InterruptMessagePtr) receivedMessage, g_Handler);
		}

		_msg_free(receivedMessage);

		// Unlock the handler for user access
		_mutex_unlock(&g_HandlerMutex);

	#ifdef PEX_USE_RTOS
	  }
	#endif
}

/*=============================================================
                    SCHEDULER INTERFACE TASK
 ==============================================================*/

void runSchedulerInterface(os_task_param_t task_init_data)
{
	//allow initializations to occur before trying to register
	OSA_TimeDelay(40);

	// Create a buffer for received messages
	char outputString[HANDLER_BUFFER_SIZE + 1];
	memset(outputString, 0, HANDLER_BUFFER_SIZE + 1);

	// Create a queue to receive messages
	_queue_id receiveQueue = _initializeQueue(SCHEDULER_QUEUE_ID);

	//Try to get read permission
	if(!OpenR(receiveQueue)){
		printf("[Scheduler Interface] SchedulerInterface OpenR() failed.\n");
		_task_block();
	}

	//This may not get used if we only use printf for feedback.
	/*
	_queue_id inputQueue = OpenW();
	//Try to get write permission
	if(inputQueue == 0){
			printf("SchedulerInterface Openw failed.\n");
			_task_block();
	}*/

#ifdef PEX_USE_RTOS
  while (1) {
#endif
	  //Listen for input from user
	  GetLine(outputString);

	  //listen for commands
	  if(!HandleCommand(outputString)){
		  printf("[Scheduler Interface] Not a valid command\n");
	  }

	  //Wipe outputString
	  memset(outputString, 0, HANDLER_BUFFER_SIZE + 1);

#ifdef PEX_USE_RTOS   
  }
#endif

	//release read
	Close();
}

/*=============================================================
                          USER TASKS
 ==============================================================*/

void doBusyWorkForTicks(uint32_t numTicks){
	MQX_TICK_STRUCT startTime;
	 _time_get_ticks(&startTime);

	 uint32_t tickDeadline = startTime.TICKS[0] + numTicks;

	MQX_TICK_STRUCT currentTime;

	do{
		_time_get_ticks(&currentTime);
	}
	while(currentTime.TICKS[0] <= tickDeadline);
}

void runUserTask(uint32_t numTicks){
	printf("[User] Doing busy work for %u ticks.\n", numTicks);
	doBusyWorkForTicks(numTicks);
	dd_delete(_task_get_id());
}

/*=============================================================
                    MONITOR TASK
 ==============================================================*/

void runMonitor(os_task_param_t task_init_data)
{
	printf("[Monitor] Monitor Task started\n");
	g_ticks = 0;
	g_milliseconds = 0;
	uint32_t ticksPerMillisecond = _time_get_ticks_per_sec()*1000;

#ifdef PEX_USE_RTOS
  while (1) {
#endif
	  	 while(g_ticks < ticksPerMillisecond){
	  		 g_ticks ++;
	  	 }
	  	 g_milliseconds ++;
	  	 g_ticks = 0;

#ifdef PEX_USE_RTOS   
  }
#endif    
}

/*=============================================================
                    STATUS UPDATE TASK
 ==============================================================*/

void runStatusUpdate(os_task_param_t task_init_data)
{
	printf("[Status Update] Status Update Task started.\n");
	uint32_t *ms_ptr = &g_milliseconds;
	while(1){
		StatusUpdate(ms_ptr);
		_time_delay(40000);
	}
//	_timer_id result = _timer_start_periodic_every(StatusUpdate,ms_ptr,TIMER_ELAPSED_TIME_MODE, 10);
//	if(result == TIMER_NULL_ID){
//		printf("[Status Update] Failed to create periodic function call\n");
//		_task_block();
//	}
}



