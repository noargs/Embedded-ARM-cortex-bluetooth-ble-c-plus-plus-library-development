#pragma once

#include <cstdarg>
#include <cstdint>
#include <cstdlib>

#include "hm10_config.hpp"
#include "main.h"

#define HM10_BUFFER_SIZE      128

namespace HM10
{

class HM10
{

public:

  HM10(UART_HandleTypeDef* huart);

  // Set/update UART configuration
  void set_uart(UART_HandleTypeDef* huart);

  UART_HandleTypeDef* UART() const;

  static constexpr supported_baudrate default_baudrate {supported_baudrate::baud_9600};

  // type alias for a callback function to handle received data
  using data_callback_t = void(*)(char*, std::size_t);

  // type alias for a callback function to handle connection events
  using device_connected_t = void(*)(mac_address const&);

  // type alias for a callback function to handle disconnection events
  using device_disconnected_t = void(*)();

  // return buffer size
  std::size_t buffer_size() const;

  // set callback for data reception
  void set_data_callback(data_callback_t callback);

  // set callback for device connection
  void set_device_conn_callback(device_connected_t callback);

  // set callback for device disconnection
  void set_device_disconn_callback(device_disconnected_t callback);

  // Boot UART
  int start_uart();

  // interrupt handlers for rx and tx
  void rx_compltd();
  void tx_compltd();

  // return busy state
  bool is_busy() const;
  bool is_rx() const;
  bool is_tx() const;

  bool is_connected() const;

  // get mac address of connected master
  mac_address master_mac() const;

  // run test command 'AT'
  bool is_alive();

  // return true if reboot successful
  bool reboot(bool waitforstartup = true);

  // return true if reset successful
  bool factory_reset(bool waitforstartup = true);

  // get/set baudrate
  supported_baudrate get_baudrate();
  bool set_baudrate(supported_baudrate new_baud, bool rebootimmediately = false, bool waitforstartup = true);

  // get/set MAC address
  mac_address get_mac_address();
  bool set_mac_address(char const* address);

  // get/set Advertisement interval
  advert_interval get_advert_interval();
  bool set_advert_interval(advert_interval interval);

  // get/set Advertisement type
  advert_type get_advert_type();
  bool set_advert_type(advert_type type);

  // get/set min and max connection interval
  conn_interval get_min_conn_interval();
  bool set_min_conn_interval(conn_interval interval);

  conn_interval get_max_conn_interval();
  bool set_max_conn_interval(conn_interval interval);

  // get/set whitelist status
  bool whitelist_en();
  bool set_whitelist_state(bool state);

  // get/set whitelist MAC
  // id: 1-3
  mac_address get_whitelisted_mac(std::uint8_t id);
  bool set_whitelisted_mac(std::uint8_t id, char const* address);

  // get/set connection slave latency
  // range: 0-4
  int conn_slave_latency();
  bool set_conn_slave_latency(int latency);

  // get/set connection supervision timeout
  conn_timeout get_conn_superv_timeout();
  bool set_conn_superv_timeout(conn_timeout timeout);

  // get/set connection updating
  bool update_conn();
  bool set_conn_updating(bool state);

  // get/set characteristics value
  std::uint16_t get_characteristics_value();
  bool set_characteristics_value(std::uint16_t value);

  // get/set notification state
  bool get_notification_state();
  bool set_notification_state(bool state);

  // clear last connected device address
  bool clear_last_connected();

  // remove bond information
  bool remove_bond_info();

  // get/set Rx gain
  bool get_rx_gain();
  bool set_rx_gain(bool gain);

  // get/set Work mode
  work_mode get_work_mode();
  bool set_work_mode(work_mode new_mode);

  // get/set automatic mode
  bool get_automatic_mode();
  bool set_automatic_mode( bool en);

  // get/set device_name
  device_name name();
  bool set_name( char const* new_name);

  // get/set output_power
  output_power get_output_power();
  bool set_output_power(output_power new_power);

  // get/set password
  // password is 6 digits long
  //  e.g. if set to 5634 it was appear as 005634
  std::uint16_t password();
  bool set_password(std::uint16_t password);

  // get/set module power
  module_power get_module_power();
  bool set_module_power( module_power new_power);

  // get/set auto sleep state
  bool get_auto_sleep();
  bool set_auto_sleep( bool en);

  // get/set reliable advertising state
  bool get_reliable_advertising();
  bool set_reliable_advertising(bool en);

  // get/set device role
  role get_role();
  bool set_role(role new_role);

  // get/set bond mode
  bond_mode get_bond_mode();
  bool set_bond_mode( bond_mode new_mode);

  // get/set service uuid value
  std::uint16_t get_service_uuid();
  bool set_service_uuid( std::uint16_t new_uuid);

  /* get/set RF communication mode */
  void set_rf_comm_mode (bool en);
  bool rf_comm_mode() const;

  // get device_version
  device_version firmware_version();

  bool start();
  bool sleep();
  bool wake_up();

  // get/set UART sleep on module shutdown
  bool uart_shutdown_on_sleep();
  bool set_uart_shutdown_on_sleep( bool state);

  // set Advertisement data
  //   12-byte maximum
  bool set_advertisement_data( char const* data);

  // send data
  bool send_data(std::uint8_t const* data, std::size_t length, bool wait_for_tx=true);



private:
  // methods for connection handling, data transmission and reception through UART

  bool handle_conn_message();

  int transmit_buff();

  void wait_for_tx_complt() const;

  void start_rx_to_buff();
  void abort_receive();
  bool receive_to_buff();

  // block until Rx completion with a default timeout
  bool wait_for_rx_complt(std::uint32_t max_time=1000) const;

  // Tx and Rx wait time
  bool tx_and_rx(std::uint32_t rx_wait_time=1000);

  // transmit data and check the response for exepected response
  bool tx_and_check_resp(char const* expected_resp, char const* format, ...);

  // copy a formatted command into buffer
  void copy_cmd_to_buff(char const* cmd_pattern, ...);

  // copy a formatted command into buffer using va_list
  void copy_cmd_to_buff_varg(char const* cmd_pattern, std::va_list args);

  // compare a string with the response
  bool compare_with_resp(char const* str) const;

  // extract a number from response message, given offset and base
  long extract_number_from_resp(std::size_t offset=7, int base=10) const;

  // copy string from response message to buffer
  void copy_str_from_resp(std::size_t offset, char* destination) const;

  // set UART baudrate
  void set_uart_baudrate(std::uint32_t new_baud) const;


  char m_txbuffer[HM10_BUFFER_SIZE] {};

  char m_rxbuffer[HM10_BUFFER_SIZE] {};

  UART_HandleTypeDef* m_uart {nullptr};

  std::size_t m_tx_datalen {0};

  /* buffer to store copied data from Rx buffer */
  char m_message_buff[HM10_BUFFER_SIZE];

  std::size_t m_message_datalen {0};

  /* pointer to start of message in Rx buffer */
  char* m_rx_buff_start_ptr {&m_rxbuffer[0]};

  /* pointer to end of message in Rx buffer */
  char* m_rx_buff_end_ptr {&m_rxbuffer[0] + HM10_BUFFER_SIZE};

  /* flag to indicate Rx in progress */
  bool m_rx_in_progress {false};

  /* flag to indicate Tx in progress */
  bool m_tx_in_progress {false};

  /* flag to indicate factory reboot pending */
  bool m_factory_reboot_pending {false};

  /* current baudrate */
  supported_baudrate m_current_baudrate {default_baudrate};

  /* new baudrate */
  supported_baudrate m_new_baudrate {default_baudrate};

  /* flag to indicate device is connected */
  bool m_is_connected {false};

  /* MAC address of connected device */
  mac_address m_connected_mac {};

  /* callback for handling data */
  data_callback_t m_data_callback {nullptr};

  /* callback for device connection */
  device_connected_t m_device_conn_callback {nullptr};

  /* callback for device disconnection */
  device_disconnected_t m_device_disconn_callback {nullptr};

  // RF communication mode
  bool m_rf_comm_mode {false};


};

}

