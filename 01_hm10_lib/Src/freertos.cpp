#include "main.h"
#include "hm10_debug.hpp"

uint32_t task_profiler;

void system_task(void* argument)
{
  while(1)
  {
//	log_info("System Init");
	debug_log("Executing");
	debug_log_level2("Executing");
	task_profiler++;
  }
}
