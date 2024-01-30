#include "main.h"

uint32_t task_profiler;

void system_task(void* argument)
{
  task_profiler++;
}
