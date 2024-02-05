#include "uart.h"

#define HM10_BAUDRATE           9600
#define DEBUG_BAUDRATE          115200

DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;


void hm10_uart_init(void)
{
  huart1.Instance = USART1;
  huart1.Init.BaudRate   = HM10_BAUDRATE;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits   = UART_STOPBITS_1;
  huart1.Init.Parity     = UART_PARITY_NONE;
  huart1.Init.Mode       = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;

  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
	// Print error
  }
}

void debug_uart_init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate   = DEBUG_BAUDRATE;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits   = UART_STOPBITS_1;
  huart2.Init.Parity     = UART_PARITY_NONE;
  huart2.Init.Mode       = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;

  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
	// Print error
  }
}

void uart_write(int ch)
{
  // Confirm data register is empty
  while(!(USART2->SR & USART_SR_TXE));

  // Write to transmit data register
  USART2->DR = (ch & 0xFF);
}

void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  if (huart->Instance == USART1)
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

	// USART Rx DMA Config
	hdma_usart1_rx.Instance = DMA2_Stream2;
	hdma_usart1_rx.Init.Channel             = DMA_CHANNEL_4;
	hdma_usart1_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
	hdma_usart1_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
	hdma_usart1_rx.Init.MemInc              = DMA_MINC_ENABLE;
	hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma_usart1_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
	hdma_usart1_rx.Init.Mode                = DMA_CIRCULAR;
	hdma_usart1_rx.Init.Priority            = DMA_PRIORITY_LOW;
	hdma_usart1_rx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;

	if (HAL_DMA_Init(&hdma_usart1_rx) != HAL_OK)
	{
	  // Print error
	}

	// Link DMA to USART1 Rx
	__HAL_LINKDMA(huart, hdmarx, hdma_usart1_rx);



	// USART Tx DMA Config
	hdma_usart1_tx.Instance = DMA2_Stream7;
	hdma_usart1_tx.Init.Channel             = DMA_CHANNEL_4;
	hdma_usart1_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
	hdma_usart1_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
	hdma_usart1_tx.Init.MemInc              = DMA_MINC_ENABLE;
	hdma_usart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma_usart1_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
	hdma_usart1_tx.Init.Mode                = DMA_CIRCULAR;
	hdma_usart1_tx.Init.Priority            = DMA_PRIORITY_LOW;
	hdma_usart1_tx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;

	if (HAL_DMA_Init(&hdma_usart1_tx) != HAL_OK)
	{
	  // Print error
	}

	// Link DMA to USART1 Rx
	__HAL_LINKDMA(huart, hdmatx, hdma_usart1_tx);


	// USART1 Interrupt PRIO
	HAL_NVIC_SetPriority(USART1_IRQn, 4, 0);
	HAL_NVIC_EnableIRQ(USART1_IRQn);


  }
  else if (huart->Instance == USART2)
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
