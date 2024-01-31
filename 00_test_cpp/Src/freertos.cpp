#include "main.h"

uint32_t task_profiler;

void system_task(void* argument)
{
  while(1)
  {
	task_profiler++;
  }
}
