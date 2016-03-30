#include "Cpu.h"
#include "Events.h"
#include "rtos_main_task.h"
#include "os_tasks.h"
#include "mqx_inc.h"

#ifdef __cplusplus
extern "C" {
#endif 

/*=============================================================
                        GLOBAL VARIABLES
 ==============================================================*/

uint64_t* volatile g_MonitorTaskTicks;
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
		{ 0, runUserTask, USER_TASK_STACK_SIZE, DEFAULT_TASK_PRIORITY, "Run Once Task", 0, 2000, 0}
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
	printf("[Scheduler] Task started.\n");

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
	printf("[Serial Handler] Task started.\n");

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
			   printf("[Serial Handler] Failed to receive a message.\n");
			   _task_block();
		}

		// Lock access to the handler while it processses the message
		if(_mutex_lock(&g_HandlerMutex) != MQX_OK){
			printf("[Serial Handler] Mutex lock failed.\n");
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
	printf("[Scheduler Interface] Task started.\n");

	// Allow scheduler and handler time to initialize
	OSA_TimeDelay(40);

	// Create a buffer for received messages
	char inputBuffer[HANDLER_BUFFER_SIZE + 1];
	memset(inputBuffer, 0, HANDLER_BUFFER_SIZE + 1);

	// Create a queue to receive messages
	_queue_id receiveQueue = _initializeQueue(SCHEDULER_QUEUE_ID);

	// Register for reading with the serial handler
	if(!OpenR(receiveQueue)){
		printf("[Scheduler Interface] Unable to register for reading with the serial handler.\n");
		_task_block();
	}

#ifdef PEX_USE_RTOS
  while (1) {
#endif
	  // Wait for console input
	  GetLine(inputBuffer);

	  // Handle input commands
	  if(!si_handleCommand(inputBuffer)){
		  printf("[Scheduler Interface] Invalid command.\n");
	  }

	  // Clear input buffer
	  memset(inputBuffer, 0, HANDLER_BUFFER_SIZE + 1);

#ifdef PEX_USE_RTOS   
  }
#endif

	// Unregister with the serial handler
	Close();
}

/*=============================================================
                          USER TASKS
 ==============================================================*/

void doBusyWorkForTicks(uint32_t numTicks){
	MQX_TICK_STRUCT currentTime;
	 _time_get_ticks(&currentTime);

	 uint32_t ticksRun = 0;
	 uint32_t currentTicks = currentTime.TICKS[0];
	 uint32_t previousTicks = currentTime.TICKS[0];

	do{
		_time_get_ticks(&currentTime);
		currentTicks = currentTime.TICKS[0];
		if(currentTicks > previousTicks){
			ticksRun++;
			previousTicks = currentTicks;
		}
	}
	while(ticksRun < numTicks);
}

void runUserTask(uint32_t numTicks){
	printf("[User] Doing busy work for %u ticks.\n", numTicks);
	doBusyWorkForTicks(numTicks);
	printf("[User] Task complete.\n");
	dd_delete(_task_get_id());
}

/*=============================================================
                    MONITOR TASK
 ==============================================================*/

typedef struct MonitorCount{
	volatile uint64_t Value;
} MonitorCount;

typedef struct MonitorData{
	MonitorCount Count;
} MonitorData;

void runMonitor(os_task_param_t task_init_data)
{
	_task_block();
	printf("[Monitor] Task started.\n");

	MonitorData monitor;
	memset(&monitor, 0, sizeof(MonitorData));
	g_MonitorTaskTicks = &monitor.Count.Value;

	#ifdef PEX_USE_RTOS
	  while (1) {
	#endif
		  if(++(&monitor)->Count.Value == 0){ // This line is exactly 5 instructions/clock cycles
			  // For reference, see https://community.freescale.com/thread/330927
		  }
	#ifdef PEX_USE_RTOS
	  }
	#endif
}

/*=============================================================
                    STATUS UPDATE TASK
 ==============================================================*/

#define FRDM_K64F_CLOCK_RATE 120e6 // 120 MHz
#define CLOCK_CYCLES_PER_IDLE_TASK_INCREMENT 5
#define MS_PER_SEC 1000

uint64_t _getIdleCount(){
    volatile KERNEL_DATA_STRUCT_PTR kernel_data;
    _GET_KERNEL_DATA(kernel_data);
    uint64_t idleCount = kernel_data->IDLE_LOOP.IDLE_LOOP2;
    idleCount = idleCount << 32;
    idleCount += kernel_data->IDLE_LOOP.IDLE_LOOP1;
    return idleCount;
}

uint32_t _getIdleMilliseconds(uint64_t idleCount){
	static uint32_t cyclesPerMs = FRDM_K64F_CLOCK_RATE / MS_PER_SEC;
	uint64_t elapsedCycles = idleCount * CLOCK_CYCLES_PER_IDLE_TASK_INCREMENT;
	return elapsedCycles / cyclesPerMs;
}

void runStatusUpdate(os_task_param_t task_init_data)
{
	printf("[Status Update] Task started.\n");

	uint64_t currentIdleCount;
	uint64_t previousIdleCount = _getIdleCount();
	uint64_t idleCountDuringPeriod;
	uint32_t inactiveMilliseconds;
	uint32_t activeMilliseconds;
	uint32_t cpuUtilization;

	while(1){
		_time_delay(STATUS_UPDATE_PERIOD);

		currentIdleCount = _getIdleCount();
		idleCountDuringPeriod = currentIdleCount - previousIdleCount;

		inactiveMilliseconds = _getIdleMilliseconds(idleCountDuringPeriod);
		activeMilliseconds = (inactiveMilliseconds > STATUS_UPDATE_PERIOD) ? STATUS_UPDATE_PERIOD : STATUS_UPDATE_PERIOD - inactiveMilliseconds;
		cpuUtilization = (activeMilliseconds * 100) / STATUS_UPDATE_PERIOD;

		printf("Active milliseconds: %u\n", activeMilliseconds);
		printf("[Status Update] CPU Utilization is: %u %% \n", cpuUtilization);

		previousIdleCount = currentIdleCount;
	}
}





