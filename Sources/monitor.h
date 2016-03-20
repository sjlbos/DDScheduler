#include <stdio.h>
#include <stdbool.h>
#include <mqx.h>
#include <message.h>
#include <mutex.h>
#include <ctype.h>
#include <string.h>

uint32_t g_ticks;
uint32_t g_milliseconds;
/*=============================================================
                      INTERNAL INTERFACE
 ==============================================================*/

/*=============================================================
                      MONITOR INTERFACE
 ==============================================================*/
uint32_t IdleTime();

