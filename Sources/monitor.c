#include <mqx.h>
#include "monitor.h"

uint32_t IdleTime(){
	uint32_t copy = g_milliseconds;
	g_milliseconds = 0;
	return copy;
}
