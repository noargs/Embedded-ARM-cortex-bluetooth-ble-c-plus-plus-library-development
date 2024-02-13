#include "main.h"
#include "hm10_debug.hpp"
#include "hm10.hpp"

#define hm10_uart                huart1

uint32_t task_profiler;

HM10::HM10 hm10(&hm10_uart);

void system_task(void* argument)
{
  int init_flag;
  bool dev_alive {false};

  /* wait for a bit */
  vTaskDelay(1000);

  /* Start uart */
  init_flag = hm10.start_uart();

  /* check if successful */
  if (init_flag != HAL_OK)
  {
	debug_log("Uart start fail!!\n");
  }

  while(!dev_alive)
  {
	dev_alive = hm10.is_alive();
	debug_log("Device Alive? %s\n", dev_alive ? "YES" : "no");
	vTaskDelay(100);
  }

  while(1)
  {
	task_profiler++;
  }
}

void uart_idle_line_callback(void)
{

}
