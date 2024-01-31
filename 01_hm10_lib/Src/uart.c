#include "uart.h"

void HAL_UART_MspInit(UART_HandleTypeDef* uarth)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  if (uarth->Instance == USART1)
  {
	// Enable clock access UART GPIO pins
	__HAL_RCC_GPIOA_CLK_ENABLE();

	// Enable clock access to UART module
	__HAL_RCC_USART1_CLK_ENABLE();

	// UART GPIO Config
	GPIO_InitStruct.Pin   = HM10_RX_PIN | HM10_TX_PIN;
	GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull  = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF7_USART1;

	HAL_GPIO_Init(HM10_PORT, &GPIO_InitStruct);
  }
  else if (uarth->Instance == USART2)
  {
	// Enable clock access UART GPIO pins
	__HAL_RCC_GPIOA_CLK_ENABLE();

	// Enable clock access to UART module
	__HAL_RCC_USART2_CLK_ENABLE();

	// UART GPIO Config
	GPIO_InitStruct.Pin   = DEBUG_RX_PIN | DEBUG_TX_PIN;
	GPIO_InitStruct.Mode  = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull  = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF7_USART2;

	HAL_GPIO_Init(DEBUG_PORT, &GPIO_InitStruct);
  }

}
