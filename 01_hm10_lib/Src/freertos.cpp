#include "main.h"
#include "hm10_debug.hpp"
#include "hm10.hpp"
#include <cstring>

#define hm10_uart                huart1

#define MSG_BUFF_SIZE            100

uint32_t task_profiler;

char message_buffer[MSG_BUFF_SIZE]{};

HM10::HM10 hm10(&hm10_uart);

bool msg_rcvd = false;

void data_callback(char* data, std::size_t length);
void device_connected_callback(HM10::mac_address const& mac);
void device_disconnected_callback();

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
	debug_log("Device Alive? %s\n", dev_alive ? "Yes" : "No");
	vTaskDelay(100);
  }

  while(1)
  {
	task_profiler++;
  }
}

// Callback function executed when a UART transmission is completed
void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart)
{
  // check if the UART handle corresponds to the HM10 modul's UART
  if (huart == &hm10_uart)
  {
	// notify the HM10 module that transmission is completed
	hm10.tx_cmpltd();
  }
}

// Callback function executed when a UART error occurs
void HAL_UART_ErrorCallback(UART_HandleTypeDef* huart)
{
  // check if the UART handle corresponds to the HM10 module's UART
  if (huart == &hm10_uart)
  {
	// print the error code in both decimal and hexadecimal formats
	debug_log("UART Comm Error - Code %d (0x%02X)\n", huart->ErrorCode, huart->ErrorCode);

  }
}

extern "C" void uart_idle_line_callback(void)
{
  // check if the UART's idle flag is set for the HM10 module's UART
  if (__HAL_UART_GET_FLAG(&hm10_uart, UART_FLAG_IDLE))
  {
	// clear the idle flag for the HM10 module's UART.
	__HAL_UART_CLEAR_IDLEFLAG(&hm10_uart);
	// notify the HM10 module that reception is completed
	hm10.rx_cmpltd();
  }
}

void data_callback(char* data, std::size_t length)
{
  // copy the received data into the global message buffer
  std::memcpy(message_buffer, data, length);

  // set the global flag indicating that a message has been received
  msg_rcvd = true;
}

// callback function executed when a device is connected
void device_connected_callback(HM10::mac_address const& mac)
{
  // print the MAC address of the connected master device
  debug_log("Connected to master with MAC address %s\n", mac.mac_address);
}

// callback function executed when a device is disconnected
void device_disconnected_callback()
{
  // print a message indicating the disconnection from the master device
  debug_log("Disconnected from device!\n");
}
