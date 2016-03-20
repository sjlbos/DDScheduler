
#include "Cpu.h"
#include "Events.h"
#include "rtos_main_task.h"
#include "os_tasks.h"
#include "TerminalDriver/handler.h"

#ifdef __cplusplus
extern "C" {
#endif 


void myUART_RxCallback(uint32_t instance, void * uartState)
{
	// Allocate an interrupt message
	InterruptMessagePtr interruptMessage = (InterruptMessagePtr)_msg_alloc(g_InterruptMessagePool);
	if (interruptMessage == NULL) {
	 _task_block();
	}

	interruptMessage->HEADER.TARGET_QID = _msgq_get_id(0, HANDLER_INTERRUPT_QUEUE_ID);
	interruptMessage->HEADER.SIZE = sizeof(InterruptMessage);
	interruptMessage->character = myRxBuff[0];

	// Send the message to the handler
	bool result = _msgq_send(interruptMessage);
	if (result != TRUE) {
		_task_block();
	}
}

#ifdef __cplusplus
}  /* extern "C" */
#endif 

