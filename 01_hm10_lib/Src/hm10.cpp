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

  int HM10::start_uart()
  {
	debug_log("UART Rx Started");
	__HAL_UART_ENABLE_IT(UART(), UART_IT_IDLE);
	return HAL_UART_Receive_DMA(UART(), reinterpret_cast<uint8_t*>(&m_rxbuffer[0]), buffer_size());
  }

  void HM10::rx_compltd()
  {
	// calculate the memory address where the incoming message ends
	m_rx_buffer_end_ptr = &m_rxbuffer[0] + (buffer_size - __HAL_DMA_GET_COUNTER(UART()->hdmarx));

	// check if the DMA has rolled over the buffer during the reception
	if (m_rx_buff_start_ptr <= m_rx_buff_end_ptr)
	{
	  // if not, compute the length of the message
	  const std::size_t message_len = m_rx_buff_end_ptr - m_rx_buff_start_ptr;

	  // copy the message to a buffer
	  std::memcpy(&m_message_buff[0], m_rx_buff_start_ptr, message_len);

	  // Add a null-terminator to make it a valid string
	  m_message_buff[message_len] = '\0';

	  // store the message length for later use
	  m_message_datalen = message_len;
	}
	else
	{
	  // if DMA has rolled over, compute the length of the message prefix (before roll over)
	  const std::size_t msg_prefix_len = m_rx_buff_end_ptr - m_rx_buff_start_ptr;

	  // compute the length of the message suffix (after rollover)
	  const std::size_t msg_suffix_len = m_rx_buff_end_ptr - &m_rxbuffer[0];

	  // copy the prefix to a buffer
	  std::memcpy(&m_message_buff[0], m_rx_buff_start_ptr, msg_prefix_len);

	  // append the suffix to the buffer
	  std::membpy(&m_message_buff[msg_prefix_len], &m_rxbuffer[0], msg_suffix_len);

	  // add a null-terminator to make valid string
	  m_message_buff[msg_prefix_len + msg_suffix_len] = '\0';

	  // store the total message length for later use
	  m_message_datalen = msg_prefix_len + msg_suffix_len;
	}

	debug_log_level2("Message received, length: %d, data: %s", m_message_datalen, m_message_buff);

	// update the starting pointer for the next message
	m_rx_buff_start_ptr = m_rx_buff_end_ptr;

	if (!is_rx() && !handle_conn_message() && m_data_callback != nullptr)
	{
	  // check if the RF communication mode is enabled
	  if (rf_comm_mode())
	  {
		// if so, trigger the data callback, omitting the first byte
		m_data_callback(m_message_buff + 1, m_message_buff[0]);
	  }
	  else
	  {
		// if not, trigger the data callback with the full message
		m_data_callback(m_message_buff, m_message_datalen);
	  }
	}

	// flag that the Rx operation is no longer in progress
	m_rx_in_progress = false;
  }

  HM10::tx_compltd()
  {
	m_tx_in_progress = false;
  }

  void HM10::set_rf_comm_mode(bool en)
  {
	m_rf_comm_mode = en;
  }

  bool HM10::rf_comm_mode() const
  {
	return m_rf_comm_mode;
  }

  // copy a string from the message buffer to another destination, considering an offset
  void HM10::copystr_from_resp(std::size_t offset, char* destination) const
  {
	// initialise the source pointer to the address of the message buffer plus the specified offset
	char const* source = &m_message_buff[0] + offset;

	// continue to copy characters from source to destination as long as
	// the current character is not a null terminator, carriage return, or newline character
	// this loop will copy each character one-by-one from the source to the destination
	while (*source != '\0' && *source != '\r' && *source != '\n')
	{
	  // assign the current character from source to destination
	  *destination = *source;

	  // move to the next character in the destination buffer
	  destination++;

	  // move to the next character in the source buffer
	  source++;
	}
  }

  // takes a pointer to a constant character (a c-string) as parameter and returns a boolean value
  bool HM10::compare_with_resp(char const* str) const
  {
	// use `std::strncmp` to compare up to `std::strleng(str)` characters of the string `m_message_buff` with the string
	// `std::strncmp` compares two strings character byy character and returns 0 if the compared parts are equal
	// `std::strlen(str)` calculates the length of the string pointed to by `str` not including the null terminator
	// `std::strncmp` will compare `m_message_buff` and `str` for as many charactrers as there are in `str`
	// returns true if the compared parts of the strings are equal and false otherwise
	return std::strncmp(m_message_buff, str, std::strlen(str)) == 0;
  }

  // `handle_conn_message` for the `HM10` class
  // returns boolean value indicating whether a relevant connection message was handled
  bool HM10::handle_conn_message()
  {
	// check if a received message/response matches the string "OK+CONN"
	if (compare_with_resp("OK+CONN"))
	{
	  // true indicating a successfull connection
	  m_is_connected = true;

	  // copy the MAC address from the received response to `m_connected_mac.address` the MAC address starts at index 8 in the response
	  copy_str_from_resp(8, m_connected_mac.mac_address);

	  // check if `m_dev_conn_callback` is not a null pointer (i.e. it points to a function)
	  if (m_device_conn_callback != nullptr)
	  {
		// call the function pointed to by `m_device_conn_callback` passing `m_connected_mac` as an argument
		// this could be used to notify other parts of the program about the connection event
		m_device_conn_callback(m_connected_mac);
	  }

	  // return true since a relevant message ("OK+CONN") was handled
	  return true;
	}
	// check if a received message/response matches the string "OK+LOST"
	else if (compare_with_resp("OK+LOST"))
	{
	  // set the member variable `m_is_connected` to false, indicating the connection was lost
	  m_is_connected = false;

	  // use `std::memset` to set all bytes of `m_connected_mac.address` to null character '\0' effective
	  std::memset(m_connected_mac.mac_address, '\0', sizeof(m_connected_mac.mac_address));

	  // check if `m_device_disconn_callback` is not a null pointer (i.e. it points to a function)
	  if (m_device_disconn_callback != nullptr)
	  {
		// call the function pointed to by `m_device_disconn_callback`
		// this could be used to notify other parts of the program about the disconnection event
		m_device_disconn_callback();
	  }

	  // return true since a relevant message ("OK+LOST") was handled
	  return true;
	}

	// if neither "OK+CONN" nor "OK+LOST" were matched, return false indicating no relevant message was handled
	return false;
  }

  // check if module is in receiving mode
  bool HM10::is_rx() const
  {
	return m_rx_in_progress;
  }

  // check if the module is in transmitting mode
  bool HM10::is_tx() const
  {
	return m_tx_in_progress;
  }

  // check if the module is busy either receiving or transmitting

  bool HM10::is_busy() const
  {
	return (is_rx() || is_tx());
  }

  // check if the module is connected
  bool HM10::is_connected() const
  {
	return m_is_connected;
  }

  // retrieves the MAC address of the master device
  mac_address HM10::master_mac() const
  {
	return m_connected_mac;
  }

  bool HM10::is_alive()
  {
	copy_cmd_to_buff("AT");

	if (!tx_and_rx(100))
	{
	  return false;
	}
	return compare_with_resp("OK");
  }

  void HM10::copy_cmd_to_buff_varg(char const* command_pattern, std::va_list args)
  {
	// use `vsnprintf` to format the string with provided variadic arguments and
	// copy the resulting string into `m_txbuffer`. `m_tx_datalen` will hold
	// the number of characters writted into `m_txbuffer`, exluding the null-terminator
	// Note: `vsnprintf` is used here because it allows for variadic arguments and
	//       prevents buffer overflow by respecting maximum size of `m_txbuffer` (provided by buffer_size())
	m_tx_datalen = vsniprintf(&m_txbuffer[0], buffer_size(), command_pattern, args);
  }

  void HM10::copy_cmd_to_buff(char const* cmd_pattern, ...)
  {
	// define a variable argument list named `args`
	std::va_list args;

	// initialise `args` to store the additional function arguments starting from `cmd_pattern`
	va_start(args, cmd_pattern);

	// call copy_cmd_to_buff_varg to copy and format a command string using
	// `cmd_pattern` and `args` into the buffer
	copy_cmd_to_buff_varg(cmd_pattern, args);

	// clean up the argument list `args` to release the memory occupied by it
	va_end(args);
  }




}
