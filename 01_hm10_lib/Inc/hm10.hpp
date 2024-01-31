#pragma once

#include <cstdarg>
#include <cstdint>

#include "hm10_config.hpp"
#include "main.h"

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
  void set_device_disconnection_callback(device_disconnected_t callback);

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
  bool reboot();

  // return true if reset successful
  bool factory_reset();
};

}

