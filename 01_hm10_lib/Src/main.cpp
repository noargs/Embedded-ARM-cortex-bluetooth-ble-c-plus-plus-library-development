#include "main.h"
#include "freertos.hpp"
#include "hm10.hpp"


#include "uart.h"

int main(void)
{
  HAL_Init();
  dma_init();
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
