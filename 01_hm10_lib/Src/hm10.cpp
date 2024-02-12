#include "hm10.hpp"
#include <cstring>
#include <cstdio>

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

  void HM10::rx_cmpltd()
  {
	// calculate the memory address where the incoming message ends
	char* const message_end_ptr = &m_rxbuffer[0] + (buffer_size() - __HAL_DMA_GET_COUNTER(UART()->hdmarx));

	// check if the DMA has rolled over the buffer during the reception
	if (m_rx_buff_start_ptr <= message_end_ptr)
	{
	  // if not, compute the length of the message
	  const std::size_t message_len = message_end_ptr - m_rx_buff_start_ptr;

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
	  const std::size_t msg_suffix_len = message_end_ptr - &m_rxbuffer[0];

	  // copy the prefix to a buffer
	  std::memcpy(&m_message_buff[0], m_rx_buff_start_ptr, msg_prefix_len);

	  // append the suffix to the buffer
	  std::memcpy(&m_message_buff[msg_prefix_len], &m_rxbuffer[0], msg_suffix_len);

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

  void HM10::tx_cmpltd()
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
	  copystr_from_resp(8, m_connected_mac.mac_address);

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

  bool HM10::tx_and_rx(std::uint32_t rx_wait_time)
  {
	// begin the receive operation
	start_rx_to_buff();

	// transmit the buffer's contents; if unsuccessful, log an error and return false
	if (transmit_buff() != 0)
	{
	  debug_log_level2("tx error");
	  return false;
	}

	// wait for the receive operation to complete; if it times out, log an error and return false
	if (wait_for_rx_cmplt(rx_wait_time) == false)
	{
	  debug_log_level2("rx timeout");
	  return false;
	}
	// if function execution reaches here, both transmit and receive were successfull
	return true;
  }

  void HM10::start_rx_to_buff()
  {
	m_rx_in_progress = true;
  }

  // `transmit_buff` attempts to transmit data using UART with DMA
  // and returns an integer indicating the result/status of the transmission attempt
  int HM10::transmit_buff()
  {
	// set a member flag `m_tx_in_progress` to true, signaling that a transmission is about to take place
	m_tx_in_progress = true;

	// log a debug message with level 2 verbosity, indicating the transmission attempt
	// and displaying the content of `m_txbuffer` that is to be transmitted
	debug_log_level2("transmitting %s", m_txbuffer);

	// vall the function `HAL_UART_Transmit_DMA to initiate the UART transmission using DMA
	// pass the UART handler, a pointer to the data to be transmitted (m_txbuffer) and the length of the data (m_tx_datalen)
	// store the result of the transmission attempt in `transmit_result`
	int transmit_result = HAL_UART_Transmit_DMA(UART(), reinterpret_cast<uint8_t*>(&m_txbuffer[0]), m_tx_datalen);

	// check if the tranmission was initiated sucessfully (transmit_result == HAL_OK)
	if (transmit_result == HAL_OK)
	{
	  wait_for_tx_cmplt();
	}
	else
	{ // if the transmission initation failed
	  // call the `tx_compltd` member function to reset flag
	  tx_cmpltd();
	}

	// return the result of the transmission attempt
	return transmit_result;
  }

  void HM10::wait_for_tx_cmplt() const
  {
	while (is_tx())
	{
	  vTaskDelay(1);
	}
  }

  bool HM10::reboot(bool waitforstartup)
  {
	copy_cmd_to_buff("AT+RESET");

	if (!tx_and_rx())
	{
	  return false;
	}

	// check if a factory reboot is pending
	if (m_factory_reboot_pending)
	{
	  // set `new_baudrate` using the array (`baudrate_values`) and the enumerator (`default_baudrate`)
	  // convert `default_baudrate` to an unsigned 8-bit integar and uses it as an index to get a value from the array
	  std::uint32_t const new_baudrate = baudrate_values[static_cast<std::uint8_t>(default_baudrate)];

	  // log a debug message indicating that a factory reboot is in progress and showing the new default baudrate
	  debug_log_level2("Factory reboot running... resetting UART baudrate to default (%d)", (int)new_baudrate);

	  // call `set_uart_baudrate` with `new_baudrate` as a parameter
	  // this function sets the UART baudrate of the HM10 module to `new_baudrate`
	  set_uart_baudrate(new_baudrate);

	  // update member variables `m_current_baudrate` and `m_new_baudrate` to `default_baudrate`
	  m_current_baudrate = default_baudrate;
	  m_new_baudrate = default_baudrate;

	  // set `m_factory_reboot_pending` to false, as the factory reboot process is now complete
	  m_factory_reboot_pending = false;
	}
	else if (m_current_baudrate != m_new_baudrate)
	{
	  // set `new_baudrate` using the array `baudrate_values` and the member variable `m_new_baudrate` as an index after
	  std::uint32_t const new_baudrate = baudrate_values[static_cast<std::uint8_t>(m_new_baudrate)];

	  // log a debug message indicating a reboot is occurring due to a baudrate change and showing the new baudrate
	  debug_log("Rebooting due to baudrate change..., new baudrate = %d", (int)new_baudrate);

	  // call `set_uart_baudrate` with `new_baudrate` as a parameter
	  set_uart_baudrate(new_baudrate);

	  // update `m_current_baudrate` to be equal to `m_new_baudrate`
	  m_current_baudrate = m_new_baudrate;
	}

	// if `wait_for_startup` is true, enter a loop to wait for the device to be responsive "alive" after rebooting
	if (waitforstartup)
	{
	  vTaskDelay(800);
	  debug_log("Starting alive check loop.");

	  // enter a loop which continues until `is_alive()` returns true
	  while (!is_alive())
	  {
		vTaskDelay(100);
	  }
	}

	// log a debug message indicating that the reboot process was successful
	debug_log_level2("Reboot successful!");
	return true;
  }


  bool HM10::factory_reset(bool waitforstartup)
  {
	if (tx_and_check_resp("OK+RENEW", "AT+RENEW"))
	{
	  m_factory_reboot_pending = true;
	  return reboot(waitforstartup);
	}
	else
	{
	  return false;
	}
  }

  bool HM10::set_baudrate(supported_baudrate new_baudrate, bool rebootimmediately, bool waitforstartup)
  {
	if (new_baudrate != supported_baudrate::baud_invalid)
	{
	  copy_cmd_to_buff("AT+BAUD%d", static_cast<std::uint8_t>(new_baudrate));
	  if (!tx_and_rx())
	  {
		return false;
	  }

	  m_new_baudrate = new_baudrate;

	  if (rebootimmediately)
	  {
		return reboot(waitforstartup);
	  }
	  return compare_with_resp("OK+SET");
	}
	return false;
  }

  /* retrieve the current baud rate setting from the HM10 module. */
  supported_baudrate HM10::get_baudrate()
  {
	return m_current_baudrate;
  }

  /* `address`: a pointer to a constant character, representing the MAC address to be set */
  bool HM10::set_mac_address(char const* address)
  {
	// log a debug message stating that the MAC address is being set
	// and include the desired MAC address (`address`) in the log message
	debug_log("Setting MAC address to %s", address);

	// call the member function `tx_and_check_resp` with parameters "OK+Set" and "AT+ADDR%s" and `address`
	// the function sends the command "AT+ADDR<address>" to the device, waits for a response
	// and checks if the reponse starts with "OK+Set"
	// retun true if the response is as expected; otherwise return false
	return tx_and_check_resp("OK+Set", "AT+ADDR%s", address);
  }

  /* retrieve the current MAC address from the HM10 module */
  mac_address HM10::get_mac_address()
  {
	mac_address addr {};

	// log a debug message indicating the intention to check the MAC address
	debug_log("Checking MAC address");

	// call the member function `tx_and_check_resp` with paramters "OK+ADDR" and "AT+ADDR?"
	// send the command "AT+ADDR?" to the device and wait for a response
	// and checks if the response starts with "OK+ADDR"
	// if `tx_and_check_resp` returns true, indicating the response was as expected
	if (tx_and_check_resp("OK+ADDR", "AT+ADDR?"))
	{
	  // call the member function `copy_str_from_resp` with parameters 8 and `addr.address`
	  // the function copies a substring from the received response message to `addr.address`
	  // starting from the 9th character (since we start counting from 0)
	  // the actual MAC address follows "OK+ADDR:" in the response hence the offset of 8 to omit this part
	  copystr_from_resp(8, addr.mac_address);
	}

	// return the `addr` instance which now contains the MAC address if the retrieval was successful
	return addr;
  }

  // function to get advertising interval
  advert_interval HM10::get_advert_interval()
  {
	debug_log("Checking advertising interval");

	// send the AT command "AT+ADVI?" to the HM10 module and check if the response start with "OK+Get"
	// `tx_and_check_resp` send the AT command to the device, waits for a response
	// and checks if the reponse matches the expected start sequence "OK+Get"
	// if it returns true (indicating the response was as expected), enter the if block
	if (tx_and_check_resp("OK+Get", "AT+ADVI?"))
	{
	  // extract a number from the response message, starting from the 8th character (index 7, counting f
	  // and interpret it as a hexadecimal number. use the extracted number to construct and `advert_interval`
	  // enumerator and return this value. the `ext_numb_from_resp` function is in charge of extracting
	  // from the message buffer, considering the offset and base as provided (7 and 16 respectively)
	  return static_cast<advert_interval>(extract_number_from_resp(7,16));
	}
	  // if the if block was not entered (meaning the response was not as expected)
	  // return an `invalid_interval` enumerator of `advert_interval` type, indicating that
	  // the advertising interval could not be determined
	  return advert_interval::advert_invalid;
  }

  // function to set advertising interval
  bool HM10::set_advert_interval(advert_interval interval)
  {
	// check if the provided advertising `interval` is not equal to `invalid_interval`
	// this ensures that an attempt to set an invalid advertising interval is not made
	if (interval != advert_interval::advert_invalid)
	{
	  // log a debug message stating the advertising interval being set
	  // convert `interval` to a `uint8_t` and insert its value into the log message
	  debug_log("Setting advertising interval to %d", static_cast<uint8_t>(interval));

	  // `tx_and_check_resp` with a formatted AT command and the desired advertising
	  // `tx_and_check_resp` sends the AT command "AT+ADVI<value>" (where <value> is the hexadecimal respresentation
	  // of `interval`) to the HM10 device, waits for a response and check if the response starts with "OK+Set"
	  // convert `interval` to a single-digit hexadecimal number and embed it in the AT command string
	  // if the response from the device matches the expectation, return true; otherwise return false
	  return tx_and_check_resp("OK+Set", "AT+ADVI%01X", static_cast<std::uint8_t>(interval));
	}
	// if the input `interval` was equal to `invalid_interval` (ensuring no attempt is made to set an invalid interval)
	// directly return false, indicating that operation wasn't successful
	return false;
  }

  advert_type HM10::get_advert_type()
  {
	// log a debug message to indicate that the function is attempting to check the advertising type
	debug_log("Checking advertising type");

	// `tx_and_check_resp` with parameters "OK+Get" and "AT+ADTY?"
	// function sends the command "AT+ADTY?" to the HM10 device, waits for a response
	// and checks if the response starts with "OK+Get"
	// if it returns true (indicating the response was expected), enter the if block
	if (tx_and_check_resp("OK+Get", "AT+ADTY?"))
	{
	  // extract a number from the response message using `ext_numb_from_resp`
	  // convert the extracted number to an `advert_type` enumerator and return this value
	  return static_cast<advert_type>(extract_number_from_resp());
	}

	// if the `if block` was not entered (meaning the response was not as expected)
	// return an `invalid` enumerator of `advert_type` type, indicating that
	// the advertising type could not be detemined
	return advert_type::advert_type_invalid;
  }

  bool HM10::set_advert_type(advert_type type)
  {
	// check if the provided advertising type (`type`) is not equal to `invalid`
	// this ensures that an attempt to set an invalid advertiding type is not made
	if (type != advert_type::advert_type_invalid)
	{
	  // log a message stating the advertising type being set
	  // convert `type` to a `uint8_t` and insert its value into the log message
	  debug_log("Setting advertising type to %d", static_cast<std::uint8_t>(type));

	  // call the member function `tx_and_check_resp` with a formatted AT command and the desired advertising
	  // `tx_and_check_resp` sends the AT command "AT+ADTY<value>" (where <value> is the integer representing
	  // of `type`) to the HM10 device, waits for a response and checks if the response starts with "OK+Set"
	  // convert `type` to an integer and embed it in the AT command string.
	  // if the response from the device matches the expectation, return value; otherwise return false
	  return tx_and_check_resp("OK+Set", "AT_ADTY%d", static_cast<std::uint8_t>(type));
	}

	// if the input `type` was equal to `invalid` (ensuring no attempt is made to set and i
	// directly return false, indicating that the operation was not successful
	return false;
  }

  conn_interval HM10::get_min_conn_interval()
  {
	debug_log("Checking minimum connection interval");
	if (tx_and_check_resp("OK+Get", "AT+COMI?"))
	{
	  return static_cast<conn_interval>(extract_number_from_resp());
	}
	return conn_interval::conn_invalid;
  }

  bool HM10::set_min_conn_interval(conn_interval interval)
  {
	if (interval != conn_interval::conn_invalid)
	{
	  debug_log("Setting minimum connection interval to %d", (int)interval);
	  return tx_and_check_resp("OK+Set", "AT+COMI%d", static_cast<std::uint8_t>(interval));
	}
	return false;
  }

  conn_interval HM10::get_max_conn_interval()
  {
	debug_log("Checking maximum connection interval");
	if (tx_and_check_resp("OK+Get", "AT+COMA?"))
	{
	  return static_cast<conn_interval>(extract_number_from_resp());
	}
	return conn_interval::conn_invalid;
  }

  bool HM10::set_max_conn_interval(conn_interval max_interval)
  {
	if (max_interval != conn_interval::conn_invalid)
	{
	  debug_log("Setting maximum connection interval to %d", (int)max_interval);
	  return tx_and_check_resp("OK+Set", "AT+COMI%d", static_cast<std::uint8_t>(max_interval));
	}
	return false;
  }

  mac_address HM10::get_whitelisted_mac(std::uint8_t id)
  {
	mac_address mac {};

	// check if `id` is not within the valid range [1, 3]
	// if it's not in range return the (empty/default) mac_address structure immediately
	if (id < 1 || id > 3)
	{
	  return mac;
	}

	debug_log("Checking whitelisted MAC #%d", id);

	// `tx_and_check_resp` returns true when provided with "OK+AD" and formatted AT
	// then proceed with the following block. this function send the AT command, "AT+AD[id]??" to the HM10
	// expecting a response that starts with "OK+AD"
	if (tx_and_check_resp("OK+AD", "AT+AD%d??", id))
	{
	  // use `copystr_from_resp` to copy a substring starting from index 8 of the response message
	  // into the `address` field of the `mac` structure. This extracts the MAC address from the response
	  copystr_from_resp(8, mac.mac_address);
	}
	// return the `mac` structure, which is either still containing the default values
	// or has been updated with the MAC address retrieved
	return mac;
  }

  bool HM10::set_whitelisted_mac(std::uint8_t id, char const* address)
  {
	debug_log("Setting whitelisted MAC #%d to %s", id, address);
	return tx_and_check_resp("OK+AD", "AT+AD%d%s", id, address);
  }

  bool HM10::get_whitelist_state()
  {
	debug_log("Checking whitelist state");

	// if the function `tx_and_check_resp` returns true when provided with "OK+Get" and "AT+ALLO?"
	// then proceed with the following block. this function sends the AT command "AT+ALLO?"
	// to the HM10 device, expecting a response that starts with "OK+Get"
	if (tx_and_check_resp("OK+Get", "AT+ALLO?"))
	{
	  // extract a number from the received message using `extract_number_from_resp()`
	  // and cast it to boolean then return it
	  // this interprets a 1 or 0 from the response as true or false, indicating the whitelist status
	  return static_cast<bool>(extract_number_from_resp());
	}
	// if the condition above is not met (i.e. the response did not start with "OK+Get")
	// execute the following block
	else
	{
	  return false;
	}
  }

  bool HM10::set_whitelist_state(bool state)
  {
	debug_log("Setting whitelist state to %d", (state ? 1 : 0));

	// call the function `tx_and_check_resp` with paramters "OK+Set" and a formatted AT command
	// use a ternary operator to embed `1` in the AT command if `status` is true and `0` otherwise
	// `tx_and_check_resp` sends the AT command "AT+ALLO<value>" to the HM10 device and expects a response
	// starting with "OK+Set" to confirm the setting action. Return the result of this function call.
	return tx_and_check_resp("OK+Set", "AT+ALLO%d", (state ? 1 : 0));
  }

  int HM10::get_slave_conn_latency()
  {
	debug_log("Checking slave connection latency");

	// transmit and check the response from the connected device
	if (tx_and_check_resp("OK+Get", "AT+COLA?"))
	{
	  return static_cast<int>(extract_number_from_resp());
	}
	return -1;
  }

  bool HM10::set_slave_conn_latency(int latency)
  {
	if (latency < 0 || latency > 4)
	{
	  return false;
	}

	debug_log("Setting slave connection latency to %d", latency);

	// transmit and check the response
	return tx_and_check_resp("OK+Set", "AT+COLA%d", static_cast<std::uint8_t>(latency));
  }

  conn_timeout HM10::get_conn_superv_timeout()
  {
	debug_log("Checking connection supervision timeout");
	if (tx_and_check_resp("OK+Get", "AT+COSU?"))
	{
	  return static_cast<conn_timeout>(extract_number_from_resp());
	}
	return conn_timeout::conn_timeout_invalid;
  }

  bool HM10::set_conn_superv_timeout(conn_timeout timeout)
  {
	if (timeout != conn_timeout::conn_timeout_invalid)
	{
	  debug_log("Setting connection supervision timeout to %d", static_cast<std::uint8_t>(timeout));
	  return tx_and_check_resp("OK+Set", "AT+COSU%d", static_cast<std::uint8_t>(timeout));
	}
	return false;
  }

  bool HM10::get_update_conn()
  {
	debug_log("Checking status of connection updating");
	if (tx_and_check_resp("OK+Get", "AT+COUP?"))
	{
	  return static_cast<bool>(extract_number_from_resp());
	}
	return false;
  }

  bool HM10::set_conn_updating(bool state)
  {
	debug_log("Setting connection updating to %d", static_cast<std::uint8_t>(state));
	return tx_and_check_resp("OK+Set", "AT+COUP%d", static_cast<std::uint8_t>(state));
  }

  std::uint16_t HM10::get_characteristics_value()
  {
	debug_log("Getting characteristics value");
	if (tx_and_check_resp("OK+Get", "AT+CHAR?"))
	{
	  return static_cast<std::uint16_t>(extract_number_from_resp(9, 16));
	}
	return 0x0000;
  }

  bool HM10::set_characteristics_value(std::uint16_t value)
  {
	if (value >= 0x0001 && value <= 0xFFFE)
	{
	  debug_log("Setting characteristics value to 0x%04X", value);
	  return tx_and_check_resp("OK+Set", "AT+CHAR0x%04X", value);
	}
	return false;
  }

  bool HM10::get_notifications_state()
  {
	debug_log("Getting notifications sate");
	if (tx_and_check_resp("OK+Get", "AT+NOTI?"))
	{
	  return static_cast<bool>(extract_number_from_resp());
	}
	return false;
  }

  bool HM10::set_notifications_state(bool enabled)
  {
	debug_log("Setting notifications state to %s", (enabled ? "true" : "false"));
	return tx_and_check_resp("OK+Set", "AT+NOTI%d", (enabled ? 1 : 0));
  }

  bool HM10::clear_last_connected()
  {
	return tx_and_check_resp("OK+CLEAR", "AT+CLEAR");
  }

  bool HM10::remove_bond_info()
  {
	return tx_and_check_resp("OK+ERASE", "AT+ERASE");
  }

  bool HM10::get_rx_gain()
  {
	debug_log("Getting Rx gain");
	if (tx_and_check_resp("OK+Get", "AT+GAIN?"))
	{
	  return static_cast<bool>(extract_number_from_resp());
	}
	return false;
  }

  bool HM10::set_rx_gain(bool gain)
  {
	debug_log("Setting Rx gain to %s", (gain ? "enabled" : "disabled"));
	return tx_and_check_resp("OK+Set", "AT+GAIN%d", (gain ? 1 : 0));
  }

  work_mode HM10::get_work_mode()
  {
	debug_log("Checking work mode");
	if (tx_and_check_resp("OK+Get", "AT+MODE?"))
	{
	  return static_cast<work_mode>(extract_number_from_resp());
	}
	return work_mode::mode_invalid;
  }

  bool HM10::set_work_mode(work_mode new_mode)
  {
	if (new_mode != work_mode::mode_invalid)
	{
	  debug_log("Setting work mode to %d", static_cast<int>(new_mode));
	  return tx_and_check_resp("OK+Set", "AT+MODE%d", static_cast<std::uint8_t>(new_mode));
	}
	return false;
  }

  bool HM10::get_automatic_mode()
  {
	debug_log("Checking if module is working in auto mode");
	if (tx_and_check_resp("OK+Get", "AT+IMME?"))
	{
	  return static_cast<bool>(extract_number_from_resp() == 0);
	}
	return false;
  }

  bool HM10::set_automatic_mode(bool enable)
  {
	debug_log("Setting auto mode to %s", (enable ? "enabled" : "disabled"));
	return tx_and_check_resp("OK+Set", "AT+IMME%d", (enable ? 0 : 1));
  }

  device_name HM10::get_name()
  {
	device_name name {};
	debug_log("Getting device name");
	if (tx_and_check_resp("OK+NAME", "AT+NAME?"))
	{
	  copystr_from_resp(8, name.name);
	}
	return name;
  }

  bool HM10::set_name(char const* new_name)
  {
	debug_log("Setting device name to %s", new_name);
	return tx_and_check_resp("OK+Set", "AT+NAME%s", new_name);
  }

  output_power HM10::get_output_power()
  {
	debug_log("Getting output power");
	if (tx_and_check_resp("OK+Get", "AT+PCTL?"))
	{
	  return static_cast<output_power>(extract_number_from_resp());
	}
	return output_power::output_power_invalid;
  }

  bool HM10::set_output_power(output_power new_power)
  {
	if (new_power != output_power::output_power_invalid)
	{
	  debug_log("Setting output power to %d", static_cast<int>(new_power));
	  return tx_and_check_resp("OK+Set", "AT+PCTL%d", static_cast<std::uint8_t>(new_power));
	}
	return false;
  }

  std::uint16_t HM10::get_password()
  {
	debug_log("Getting the pairing password");
	if (tx_and_check_resp("OK+Get", "AT+PASS?"))
	{
	  return static_cast<std::uint16_t>(extract_number_from_resp());
	}
	return std::uint16_t {};
  }

  bool HM10::set_password(std::uint16_t new_pass)
  {
	if (new_pass <= 999999)
	{
	  debug_log("Setting the password to %06d", (int)new_pass);
	  return tx_and_check_resp("OK+Set", "AT+PASS%06d", new_pass);
	}
	return false;
  }

  module_power HM10::get_module_power()
  {
	debug_log("Getting module power");
	if (tx_and_check_resp("OK+Get", "AT+POWE?"))
	{
	  return static_cast<module_power>(extract_number_from_resp());
	}
	return module_power::power_invalid;
  }

  bool HM10::set_module_power(module_power new_power)
  {
	if (new_power != module_power::power_invalid)
	{
	  debug_log("Setting module power to %d", static_cast<std::uint8_t>(new_power));
	  return tx_and_check_resp("OK+Set", "AT+POWE%d", static_cast<std::uint8_t>(new_power));
	}
	return false;
  }

  bool HM10::get_auto_sleep()
  {
	debug_log("Checking auto sleep state");
	if (tx_and_check_resp("OK+Get", "AT+PWRM?"))
	{
	  return static_cast<bool>(extract_number_from_resp());
	}
	return false;
  }

  bool HM10::set_auto_sleep(bool enabled)
  {
	debug_log("Setting auto sleep state to %s", (enabled ? "enabled" : "disabled"));
	return tx_and_check_resp("OK+Set", "AT+PWRM%d", (enabled ? 0:1));
  }

  bool HM10::get_reliable_advertising( )
  {
	debug_log("Checking reliable advertising mode");
	if (tx_and_check_resp("OK+Get", "AT+RELI?"))
	{
	  return static_cast<bool>(extract_number_from_resp());
	}
	return false;
  }

  bool HM10::set_reliable_advertising(bool enabled)
  {
	debug_log("Setting reliable advertising mode to %s", (enabled ? "enabled" : "disabled"));
	return tx_and_check_resp("OK+Set", "AT+RELI%d", (enabled ? 1 : 0));
  }

  role HM10::get_role()
  {
	debug_log("Getting device role");
	if (tx_and_check_resp("OK+Get", "AT+ROLE?"))
	{
	  return static_cast<role>(extract_number_from_resp());
	}
	return role::role_invalid;
  }

  bool HM10::set_role(role new_role)
  {
	debug_log("Setting device role to %d", static_cast<int>(new_role));
	return tx_and_check_resp("OK+Set", "AT+ROLE%d", static_cast<std::uint8_t>(new_role));
  }

  bool HM10::start()
  {
	debug_log("Starting the module");
	return tx_and_check_resp("OK+START", "AT+START");
  }

  bool HM10::sleep()
  {
	debug_log("Putting the module into sleep mode");
	return tx_and_check_resp("OK+SLEEP", "AT+SLEEP");
  }

  bool HM10::wake_up()
  {
	debug_log("Waking the module up");
	return tx_check_resp("OK+WAKE", "WAKEUP");
  }

  std::uint16_t HM10::get_service_uuid()
  {
	debug_log("Getting service UUID");
	if (tx_and_check_resp("OK+Get", "AT+UUID?"))
	{
	  return static_cast<std::uint16_t>(extract_number_from_resp(9, 16));
	}
	return 0x0000;
  }

  bool HM10::set_service_uuid(std::uint16_t new_uuid)
  {
	if (new_uuid >= 0x0001 && new_uuid <= 0xFFFE)
	{
	  debug_log("Setting service UUID to 0x%04X", new_uuid);
	  return tx_and_check_resp("OK+Set", "AT+UUID0x%04X", new_uuid);
	}
	return false;
  }

  bool HM10::set_advertisement_data(char const* data)
  {
	debug_log("Setting advertisement data to %s", data);
	return tx_and_check_resp("OK+Set", "AT+PACK%s", data);
  }

  device_version HM10::firmware_version()
  {
	device_version ver {};
	debug_log("Getting firmware version");
	copy_cmd_to_buff("AT+VERR?");
	if (tx_and_rx())
	{
	  copystr_from_resp(0, ver.version);
	}
	return ver;
  }

  bool HM10::send_data(std::uint8_t const* data, std::size_t length, bool wait_for_tx)
  {
	// check if a connection is established by calling the `is_connected` member function
	// if true, proceed with the next block, else skip to the return false statement at the end
	if (is_connected())
	{
	  // set a member flag `m_tx_in_progress` to true, indicating that a transmission is about to take place
	  m_tx_in_progress = true;

	  // call the function `HAL_UART_Transmit_DMA` to initiate UART transmission using DMA.
	  // pass the UART handler, the data pointer (with const cast) and the length of the data
	  // store the result of the transmission attempt (likely a status code) in `transmit_reslut`
	  int transmit_result = HAL_UART_Transmit_DMA(UART(), const_cast<std::uint8_t*>(data), length);

	  // check if the transmission was initiated successfully (transmit_result == HAL_OK)
	  if (transmit_result == HAL_OK)
	  {
		// if `wait_for_tx` is true, call the `wait_for_tx_cmplt` member function
		// which might suspend the current task until the transmission is complete
		if (wait_for_tx)
		{
		  wait_for_tx_cmplt();
		}

		// return true indicating that the data was sent (or is being sent if wait_for_tx is false)
		return true;
	  }
	  else // if the transmission initiation failed
	  {
		// call the `tx_cmpltd` member function, to reset m_tx_in_progress flag
		// or handle the transmission
		tx_cmpltd();
	  }
	}
	// return false either if the device is not connected
	// or if the transmission initiation failed
	return false;
  }



}




















