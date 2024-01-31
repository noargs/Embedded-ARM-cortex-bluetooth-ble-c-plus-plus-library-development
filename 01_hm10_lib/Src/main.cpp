#include "main.h"
#include "freertos.hpp"


#include "uart.h"

int main(void)
{
  HAL_Init();

  hm10_uart_init();
  debug_uart_init();

  xTaskCreate(system_task,
			  "system task",
			  100,
			  NULL,
			  1,
			  NULL);

  vTaskStartScheduler();

  while(1)
  {

  }
}
