#include "hm10.hpp"

namespace HM10
{

  /* construct HM10 object and init uart handle `huart` */
  HM10::HM10(UART_HandleTypeDef* huart)
  {
	if (huart != nullptr) set_uart(huart);
  }

  /* set UART handle `huart` */
  void HM10::set_uart(UART_HandleTypeDef* huart)
  {
	m_uart = huart;
  }

  /* get UART handle */
  UART_HandleTypeDef* HM10::UART() const
  {
	return m_uart;
  }

  /* get buffer size */
  std::size_t HM10::buffer_size() const
  {
	return HM10_BUFFER_SIZE;
  }

  /* sets the data callback function */
  void HM10::set_data_callback(data_callback_t callback)
  {
	m_data_callback = callback;
  }

  /* sets the device connected callback function */
  void HM10::set_device_conn_callback(device_connected_t callback)
  {
	m_device_conn_callback = callback;
  }

  /* sets the device disconneted callback function */
  void HM10::set_device_disconn_callback(device_disconnected_t callback)
  {
	m_device_disconn_callback = callback;
  }

}
