#include <stdio.h>
#include <stdbool.h>
#include <mqx.h>
#include <message.h>
#include <mutex.h>
#include <ctype.h>

#ifndef SOURCES_HANDLER_H_
#define SOURCES_HANDLER_H_

/*=============================================================
                         CONSTANTS
 ==============================================================*/

#define PRINT_DELAY_MS 10

#define HANDLER_INTERRUPT_QUEUE_ID 8
#define HANDLER_INPUT_QUEUE_ID 9

#define HANDLER_BUFFER_SIZE 256
#define HANDLER_READER_MAX 32

#define SERIAL_MESSAGE_POOL_INITIAL_SIZE 16
#define SERIAL_MESSAGE_POOL_GROWTH_RATE 16
#define SERIAL_MESSAGE_POOL_MAX_SIZE 2048

#define INTERRUPT_MESSAGE_POOL_INITIAL_SIZE 1
#define INTERRUPT_MESSAGE_POOL_GROWTH_RATE 1
#define INTERRUPT_MESSAGE_POOL_MAX_SIZE 16

/*=============================================================
                      EXPORTED TYPES
 ==============================================================*/

// Defines a character buffer used by the handler
typedef struct HandlerBuffer {
	int currentSize;
	int maxSize;
	char* characters;
} HandlerBuffer, * HandlerBufferPtr;

// Defines a binding between the handler and a user task with read access
typedef struct HandlerReader{
	_task_id taskId;
	_queue_id queueId;
} HandlerReader, *HandlerReaderPtr;

// Defines a list of HandlerReader objects
typedef struct HandlerReaderList{
	int count;
	int maxSize;
	HandlerReaderPtr* readers;
} HandlerReaderList, * HandlerReaderListPtr;

// Defines a structure for maintaining the handler's internal state
typedef struct Handler{
	HandlerReaderList readerList;
	HandlerBuffer buffer;
	_task_id currentWriter;
	_queue_id charInputQueue;
	_queue_id bufferInputQueue;
	uint32_t terminalInstance;
} Handler, * HandlerPtr;

// Defines a generic message pointer used to resolve anonymous message pointers
typedef struct GenericMessage{
	MESSAGE_HEADER_STRUCT HEADER;
} GenericMessage, * GenericMessagePtr;

// Defines a message sent between the handler and a user task
typedef struct SerialMessage{
	MESSAGE_HEADER_STRUCT HEADER;
	int length;
	char* content;
} SerialMessage, * SerialMessagePtr;

// Defines a message sent from the UART ISR to the handler
typedef struct InterruptMessage
{
   MESSAGE_HEADER_STRUCT   HEADER;
   uint8_t character;
} InterruptMessage, * InterruptMessagePtr;

/*=============================================================
                      GLOBAL VARIABLES
 ==============================================================*/

extern _pool_id g_InterruptMessagePool;		// A message pool for messages sent between from the UART event handler to the handler task
extern _pool_id g_SerialMessagePool;		// A message pool for messages sent between the handler task and its user tasks
extern HandlerPtr g_Handler;				// The global handler instance
extern MUTEX_STRUCT g_HandlerMutex;			// The mutex controlling access to the handler's internal state

/*=============================================================
                      USER TASK INTERFACE
 ==============================================================*/

bool OpenR(uint16_t streamNumber);
bool GetLine(char* outputString);
_queue_id OpenW(void);
bool PutLine(_queue_id queueId, char* inputString);
bool Close(void);

/*=============================================================
                      INTERNAL INTERFACE
 ==============================================================*/

void _initializeHandler(HandlerPtr handler, _queue_id charInputQueue, _queue_id bufferInputQueue, uint32_t terminalInstance);
void _initializeHandlerMutex(MUTEX_STRUCT* mutex);
void _handleInterruptMessage(InterruptMessagePtr interruptMessage, HandlerPtr handler);
void _handleWriteMessage(SerialMessagePtr serialMessage, HandlerPtr handler);

#endif
