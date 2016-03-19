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

void _handleActive();
void _handlerOverdue();
void _handleCPUUtilization();
void _prettyPrintTaskList();

/*=============================================================
                      STATUS UPDATE INTERFACE
 ==============================================================*/

void StatusUpdate();
