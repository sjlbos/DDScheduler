#include <stdio.h>
#include <stdbool.h>
#include <mqx.h>
#include <message.h>
#include <mutex.h>
#include <ctype.h>
#include <string.h>

/*=============================================================
                      INTERNAL INTERFACE
 ==============================================================*/

_task_id _handleCreate(char* outputString);
bool _handleDelete(char* outputString);
void _handleActive();
void _handlerOverdue();
void _prettyPrintTaskList();
_task_id _create_periodic(uint32_t templateIndex, uint32_t deadline)
/*=============================================================
                      SCHEDULER INTERFACE
 ==============================================================*/
bool HandleCommand(char* outputString);
