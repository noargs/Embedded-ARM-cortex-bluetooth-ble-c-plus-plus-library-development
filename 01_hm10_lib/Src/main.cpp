#include "main.h"
#include "freertos.hpp"

int main(void)
{
  HAL_Init();

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
