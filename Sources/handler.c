#include "handler.h"

/*=============================================================
                      INITIALIZATION
 ==============================================================*/

void _initializeHandlerBuffer(HandlerBufferPtr handlerBuffer){
	char* charBuffer;
	if(!(charBuffer = (char*) malloc(sizeof(char) * (HANDLER_BUFFER_SIZE + 1)))){
		printf("Unable to allocate memory for character buffer.");
		_task_block();
	}
	memset(charBuffer, 0, sizeof(char) * (HANDLER_BUFFER_SIZE + 1));

	handlerBuffer->currentSize = 0;
	handlerBuffer->maxSize = HANDLER_BUFFER_SIZE;
	handlerBuffer->characters = charBuffer;
}

void _initializeHandlerReaderList(HandlerReaderListPtr readerList){
	HandlerReaderPtr* readers;
	if(!(readers = (HandlerReaderPtr*) malloc(sizeof(HandlerReaderPtr) * HANDLER_READER_MAX))){
		printf("Unable to allocate memory for reader list.");
		_task_block();
	}
	memset(readers, 0, sizeof(HandlerReaderPtr) * HANDLER_READER_MAX);

	readerList->count = 0;
	readerList->maxSize = HANDLER_READER_MAX;
	readerList->readers = readers;
}

void _initializeHandler(HandlerPtr handler, _queue_id charInputQueue, _queue_id bufferInputQueue, uint32_t terminalInstance){
	_initializeHandlerBuffer(&handler->buffer);
	_initializeHandlerReaderList(&handler->readerList);
	handler->currentWriter = 0;
	handler->charInputQueue = charInputQueue;
	handler->bufferInputQueue = bufferInputQueue;
	handler->terminalInstance = terminalInstance;
}

void _initializeHandlerMutex(MUTEX_STRUCT* mutex){
	MUTEX_ATTR_STRUCT handlerMutexAttributes;
	if(_mutatr_init(&handlerMutexAttributes) != MQX_OK){
		printf("Mutex attribute initialization failed.\n");
		_task_block();
	}

	if(_mutex_init(mutex, &handlerMutexAttributes) != MQX_OK){
		printf("Mutex initialization failed.\n");
		_task_block();
	}
}

/*=============================================================
                          MESSAGES
 ==============================================================*/

SerialMessagePtr _initializeSerialMessage(char* message, _queue_id destination){
	SerialMessagePtr serialMessage = (SerialMessagePtr)_msg_alloc(g_SerialMessagePool);
	if (serialMessage == NULL) {
	 printf("Could not allocate a message.\n");
	 _task_block();
	}

	int messageSize = strlen(message);

	// Allocate a new message string
	char* messageCopy;
	if(!(messageCopy = (char*) malloc(sizeof(char) * messageSize))){
		printf("Could not allocate a message copy.");
		_task_block();
	}

	strncpy(messageCopy, message, messageSize);

	serialMessage->HEADER.SIZE = sizeof(SerialMessage);
	serialMessage->HEADER.TARGET_QID = destination;
	serialMessage->length = messageSize;
	serialMessage->content = messageCopy;

	return serialMessage;
}

/*=============================================================
                      READER MANAGEMENT
 ==============================================================*/

bool _addHandlerReader(_task_id taskId, _queue_id queue, HandlerPtr handler){

	int currentReaderCount = handler->readerList.count;

	if(currentReaderCount == handler->readerList.maxSize){
		return false;
	}

	HandlerReaderPtr reader;
	if(!(reader = (HandlerReaderPtr) malloc(sizeof(HandlerReader)))){
		printf("Unable to allocate memory for HandlerReader.");
		_task_block();
	}

	reader->queueId = queue;
	reader->taskId = taskId;

	handler->readerList.readers[currentReaderCount] = reader;
	handler->readerList.count++;

	return true;
}

bool _clearHandlerReader(_task_id taskId, HandlerPtr handler){
	int numReaders = handler->readerList.count;
	HandlerReaderPtr* readers = handler->readerList.readers;

	for(int i=0; i<numReaders; i++){
		if(readers[i]->taskId == taskId){

			// Deallocate the reader
			free(readers[i]);
			readers[i] = NULL;

			// Shift the remaining readers left by 1 index
			for (int j=i; j<numReaders-1; j++){
				readers[j] = readers[j+1];
			}
			handler->readerList.count--;
			return true;
		}
	}
	return false;
}

_queue_id _getReaderQueueNum(_task_id taskId, HandlerPtr handler){
	int numReaders = handler->readerList.count;
	HandlerReaderPtr* readers = handler->readerList.readers;

	for(int i=0; i<numReaders; i++){
		if(readers[i]->taskId == taskId){
			return readers[i]->queueId;
		}
	}

	return MSGQ_NULL_QUEUE_ID;
}

void _writeMessageToReaders(char* message, HandlerReaderListPtr readerList){
	SerialMessagePtr serialMessage;
	for(int i=0; i<readerList->count; i++){
		serialMessage = _initializeSerialMessage(message, readerList->readers[i]->queueId);
		bool result = _msgq_send(serialMessage);
		if (result != TRUE){
			printf("Failed to send message to reader %d.\n", readerList->readers[i]->taskId);
			_task_block();
		}
	}
}

/*=============================================================
                      WRITER MANAGEMENT
 ==============================================================*/

bool _clearHandlerWriter(_task_id taskId, HandlerPtr handler){
	if(handler->currentWriter == taskId){
		handler->currentWriter = 0;
		return true;
	}
	return false;
}

/*=============================================================
                      BUFFER MANAGEMENT
 ==============================================================*/

static const char BackspaceString[] = "\b \b";
static const int BackspaceStringLen = 3;

void _printCharacterToTerminal(char character, uint32_t terminalId){
	UART_DRV_SendData(terminalId, &character, sizeof(char));
	OSA_TimeDelay(PRINT_DELAY_MS);
}

void _printStringToTerminal(char* string, int size, uint32_t terminalId){
	UART_DRV_SendData(terminalId, string, sizeof(char) * size);
	OSA_TimeDelay(PRINT_DELAY_MS);
}

bool _addCharacterToEndOfBuffer(char character, HandlerBufferPtr buffer){
	int currentSize = buffer->currentSize;
	if(currentSize == buffer->maxSize){
		return false;
	}
	buffer->characters[currentSize] = character;
	buffer->currentSize++;
	return true;
}

char _removeCharacterFromEndOfBuffer(HandlerBufferPtr buffer){
	int currentSize = buffer->currentSize;
	if(currentSize == 0){
		return '\0';
	}
	char removedChar = buffer->characters[currentSize-1];
	buffer->characters[currentSize-1] = '\0';
	buffer->currentSize--;

	return removedChar;
}

char _getLastCharacterInBuffer(HandlerBufferPtr buffer){
	if(buffer->currentSize == 0){
		return '\0';
	}
	return buffer->characters[buffer->currentSize - 1];
}

void _clearBuffer(HandlerBufferPtr buffer){
	memset(buffer->characters, 0, sizeof(char) * buffer->maxSize + 1);
	buffer->currentSize = 0;
}

void _handleNewline(HandlerPtr handler){
	_printStringToTerminal("\r\n", 2, handler->terminalInstance);
	if (handler->buffer.currentSize > 0){
		_addCharacterToEndOfBuffer('\r', &handler->buffer);
		_addCharacterToEndOfBuffer('\n', &handler->buffer);
		_writeMessageToReaders(handler->buffer.characters, &handler->readerList);
		_clearBuffer(&handler->buffer);
	}
}

void _handleBackspace(HandlerPtr handler){
	_removeCharacterFromEndOfBuffer(&handler->buffer);
	_printStringToTerminal(BackspaceString, BackspaceStringLen, handler->terminalInstance);
}

void _handleEraseLine(HandlerPtr handler){

	// Create character string to delete terminal line in one step
	int whitespaceStringLen = handler->buffer.currentSize * BackspaceStringLen;
	char whitespaceString[whitespaceStringLen];
	for(int i=0; i<whitespaceStringLen; i+=BackspaceStringLen){
		for(int j=0; j<BackspaceStringLen; j++){
			whitespaceString[i+j] = BackspaceString[j];
		}
	}
	_printStringToTerminal(whitespaceString, whitespaceStringLen, handler->terminalInstance);
	_clearBuffer(&handler->buffer);
}

void _handleEraseWord(HandlerPtr handler){
	HandlerBufferPtr buffer = &handler->buffer;

	char lastChar = _getLastCharacterInBuffer(buffer);

	// Clear any leading whitespace
	while(lastChar == ' '){
		_printStringToTerminal("\b \b",  3, handler->terminalInstance);
		_removeCharacterFromEndOfBuffer(buffer);
		lastChar = _getLastCharacterInBuffer(buffer);
	}

	// Clear non-space characters until a space character is encountered or the end of the buffer is reached
	while(lastChar != ' ' && lastChar != '\0'){
		_printStringToTerminal("\b \b",  3, handler->terminalInstance);
		_removeCharacterFromEndOfBuffer(buffer);
		lastChar = _getLastCharacterInBuffer(buffer);
	}
}

void _handleTab(HandlerPtr handler){

}

void _handleBell(HandlerPtr handler){

}

void _handleEscape(HandlerPtr handler){

}

void _handleRegularCharacter(char character, HandlerPtr handler){
	_addCharacterToEndOfBuffer(character, &handler->buffer);
	_printCharacterToTerminal(character, handler->terminalInstance);
}

void _handleCharacterInput(char character, HandlerPtr handler){
	switch(character){
		case 0x0A: // \n
		case 0x0D: // \r
			_handleNewline(handler);
			break;
		case 0x08: // Backspace
			_handleBackspace(handler);
			break;
		case 0x15: // CTRL U
			_handleEraseLine(handler);
			break;
		case 0x17: // CTRL W
			_handleEraseWord(handler);
			break;
		case 0x09: // Tab
			_handleTab(handler);
			break;
		case 0x07: // Bell
			_handleBell(handler);
			break;
		case 0x1B: // Esc
			_handleEscape(handler);
			break;
		default:
			if(isprint((unsigned char)character)){
				_handleRegularCharacter(character, handler);
			}
			break;
	}
}

void _handleInterruptMessage(InterruptMessagePtr interruptMessage, HandlerPtr handler){
	char inputChar = interruptMessage->character;
	_handleCharacterInput(inputChar, g_Handler);
	_msg_free(interruptMessage);
}

void _handleWriteMessage(SerialMessagePtr serialMessage, HandlerPtr handler){
	char* messageString = serialMessage->content;

	for(int i=0; i < serialMessage->length; i++){
		_handleCharacterInput(messageString[i], handler);
	}
	_msg_free(serialMessage);
}

/*=============================================================
                      USER TASK INTERFACE
 ==============================================================*/

bool OpenR(uint16_t streamNumber){
	if(_mutex_lock(&g_HandlerMutex) != MQX_OK){
		printf("Mutex lock failed.\n");
		_task_block();
	}

	_task_id thisTask = _task_get_id();

	// Ensure this task does not already have read privileges
	if (_getReaderQueueNum(thisTask, g_Handler) != MSGQ_NULL_QUEUE_ID){
		_mutex_unlock(&g_HandlerMutex);
		return false;
	}

	// Register this task for reading with the handler
	bool result = _addHandlerReader(thisTask, streamNumber, g_Handler);

	_mutex_unlock(&g_HandlerMutex);
	return result;
}

bool GetLine(char* outputString){
	if(outputString == NULL){
		return false;
	}

	// Get the reader queue for this task
	if(_mutex_lock(&g_HandlerMutex) != MQX_OK){
		printf("Mutex lock failed.\n");
		_task_block();
	}
	_queue_id readerQueue = _getReaderQueueNum(_task_get_id(), g_Handler);
	_mutex_unlock(&g_HandlerMutex);

	// Ensure this task has read privileges
	if(readerQueue == MSGQ_NULL_QUEUE_ID){
		return false;
	}

	// Wait for the next message to arrive
  	SerialMessagePtr message = _msgq_receive(readerQueue, 0);
	if (message == NULL) {
	   printf("Could not receive a message\n");
	   _task_block();
	}

	// Copy message to output string
	strncpy(outputString, message->content, message->length);

	// Dispose of message
	free(message->content);
	_msg_free(message);

	return true;
}

_queue_id OpenW(void){
	if(_mutex_lock(&g_HandlerMutex) != MQX_OK){
		printf("Mutex lock failed.\n");
		_task_block();
	}

	_task_id writer = g_Handler->currentWriter;

	if (writer != 0){
		_mutex_unlock(&g_HandlerMutex);
		return 0;
	}

	g_Handler->currentWriter = _task_get_id();
	_queue_id inputQueue = g_Handler->bufferInputQueue;

	_mutex_unlock(&g_HandlerMutex);
	return inputQueue;
}

bool PutLine(_queue_id queueId, char* inputString){
	// Get current writer
	if(_mutex_lock(&g_HandlerMutex) != MQX_OK){
		printf("Mutex lock failed.\n");
		_task_block();
	}

	_task_id currentWriter = g_Handler->currentWriter;

	_mutex_unlock(&g_HandlerMutex);

	// Check that current task has write access
	if(currentWriter != _task_get_id()){
		return false;
	}

	// Initialize serial message
	SerialMessagePtr writeMessage = _initializeSerialMessage(inputString, queueId);

	// Write serial message to queue
	if (!_msgq_send(writeMessage)) {
		printf("Could not send a message.\n");
		_task_block();
	}

	return true;
}

bool Close(void){
	if(_mutex_lock(&g_HandlerMutex) != MQX_OK){
		printf("Mutex lock failed.\n");
		_task_block();
	}

	_task_id thisTask = _task_get_id();

	bool closeResult = _clearHandlerReader(thisTask, g_Handler) || _clearHandlerWriter(thisTask, g_Handler);

	_mutex_unlock(&g_HandlerMutex);

	return closeResult;
}


