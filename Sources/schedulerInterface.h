#include <stdio.h>
#include <stdbool.h>
#include <mqx.h>
#include <message.h>
#include <mutex.h>
#include <ctype.h>
#include <string.h>


/*=============================================================
                      SCHEDULER INTERFACE
 ==============================================================*/
bool si_handleCommand(char* commandString);

