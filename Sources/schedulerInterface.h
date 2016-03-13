#include <stdio.h>
#include <stdbool.h>
#include <mqx.h>
#include <message.h>
#include <mutex.h>
#include <ctype.h>

/*=============================================================
                      INTERNAL INTERFACE
 ==============================================================*/
void _handleCreate();
void _handleDelete();
void _handleActive();
void _handlerOverdue();


/*=============================================================
                      SCHEDULER INTERFACE
 ==============================================================*/
bool HandleCommand(char* outputString);
