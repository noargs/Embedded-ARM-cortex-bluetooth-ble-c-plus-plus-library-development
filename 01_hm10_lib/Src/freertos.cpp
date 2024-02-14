#include "main.h"
#include "hm10_debug.hpp"
#include "hm10.hpp"
#include <cstring>
#include <stdlib.h>
#include <test_modules.hpp>

#define hm10_uart                huart1

#define MSG_BUFF_SIZE            100

uint32_t task_profiler;

char message_buffer[MSG_BUFF_SIZE]{};

HM10::HM10 hm10(&hm10_uart);

bool msg_rcvd = false;

void data_callback(char* data, std::size_t length);
void device_connected_callback(HM10::mac_address const& mac);
void device_disconnected_callback();

char* ble_rx_data;

// converts ASCII characters in a buffer to their hexadecimal values
char* buffer_to_hex(const char* buffer, size_t length)
{
  char* result = (char*)malloc(length * 2+1); // two hex chars per byte + null terminator
  if (!result)
  {
	fprintf(stderr, "Failed to allocate memory for hex string\n");
	exit(1);
  }

  const char* hex_chars = "0123456789ABCDEF";
  for (size_t i=0; i<length; i++)
  {
	result[i*2] = hex_chars[(buffer[i] & 0xF0) >> 4];
	result[i*2+1] = hex_chars[buffer[i] & 0x0F];
  }
  result[length*2] = '\0';                   // null-terminate the hex string
  return result;
}

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

  hm10.set_data_callback(data_callback);
  hm10.set_device_connected_callback(device_connected_callback);
  hm10.set_device_disconnected_callback(device_disconnected_callback);

  /* some tests */
  HM10::device_version version = hm10.firmware_version();
  printf("Firmware version: %s\n", version.version);

  hm10.set_work_mode(HM10::work_mode::mode_tx);
  printf("Work mode: %d/n", static_cast<int>(hm10.get_work_mode()));

  HM10::device_name name = hm10.get_name();
  printf("Name: %s\n", name.name);

  hm10.set_name("NOARGS-IBN");
  name = hm10.get_name();
  printf("New name %s\n", name.name);

  hm10.set_service_uuid(0xDEAD);
  printf("Service UUID: 0x%04X\n", hm10.get_service_uuid());

  hm10.set_characteristics_value(0xBEEF);
  printf("Characteristics value: 0x%04X\n", hm10.get_characteristics_value());

  hm10.set_notifications_state(true);
  printf("Notifications state: %s\n", hm10.get_notifications_state() ? "enabled" : "disabled");

  // "Hello STM32" will be received by LightBlue App as Hexadecimal string which you will have to parse
  char const* tx_data = "Hello STM32";
  char const* resp = "Hi";

  while(1)
  {
	/* Uncomment to test sending data to master (HM10) */
	//hm10.send_data((uint8_t const*)tx_data, std::strlen(tx_data));
	//vTaskDelay(10);

	if (msg_rcvd)
	{
	  ble_rx_data = buffer_to_hex(message_buffer, std::strlen(message_buffer));
	  printf("Master sent %d bytes, data: %s\n", std::strlen(message_buffer), ble_rx_data);
	  hm10.send_data((uint8_t const*)resp, std::strlen(resp));
	  msg_rcvd = false;
	}

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
