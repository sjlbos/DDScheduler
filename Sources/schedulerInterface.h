#include <stdio.h>
#include <stdbool.h>
#include <mqx.h>
#include <message.h>
#include <mutex.h>
#include <ctype.h>
#include <string.h>

#ifndef _SCHEDULER_INTERFACEH_
#define _SCHEDULER_INTERFACEH_

/*=============================================================
                      STRUCT DEFINITIONS
 ==============================================================*/

typedef struct PeriodicTaskMapping{
	_task_id periodicGeneratorID;
	_task_id userTaskID;
} PeriodicTaskMapping, *PeriodicTaskMappingPtr;

typedef struct PeriodicTaskParameterList{
	uint32_t TemplateIndex;
	uint32_t Deadline;
	uint32_t Period;
	PeriodicTaskMappingPtr periodicTaskMap;
} PeriodicTaskParameterList, *PeriodicTaskParameterListPtr;

/*=============================================================
                      SCHEDULER INTERFACE
 ==============================================================*/
bool si_handleCommand(char* commandString);


#endif
