
#include "test_modules.hpp"
#include "hm10_debug.hpp"

#define hm10_uart_test            huart1

// Create an instance of HM10 called hm10_test, initializing it with the address of the HM10_UART variable
HM10::HM10 hm10_test(&hm10_uart_test);


// Define a function to test the factory reset feature of the hm10_test instance
void test_factory_reset() {
	hm10_test.factory_reset();
	debug_log("Is alive after factory reboot? %s\n", (hm10_test.is_alive() ? "yes" : "no"));
}

// Define a function to test the baud rate setting of the hm10_test instance
void test_baud_rate(HM10::supported_baudrate new_baud) {
	hm10_test.set_baudrate(new_baud);

	debug_log("Is alive after baudrate change? %s\n", (hm10_test.is_alive() ? "yes" : "no"));
}

// Define a function to test MAC address retrieval and setting on the hm10_test instance
void test_mac_address(char const* new_mac) {
    // Get the current MAC address of hm10_test and store it in address
  HM10::mac_address address = hm10_test.get_mac_address();
  debug_log("Current MAC address: %s\n", address.mac_address);

  // Check if the length of the new MAC address string is valid (12 characters)
  if (std::strlen(new_mac) == 12) {
      // Set the MAC address of hm10_test to new_mac
	  hm10_test.set_mac_address(new_mac);
    address = hm10_test.get_mac_address();
    debug_log("New MAC address: %s\n", address.mac_address);
  }
}

void test_advert_interval(HM10::advert_interval new_interval) {
	debug_log("Current advert interval: 0x%01X\n", hm10_test.get_advert_interval());

  if (new_interval != HM10::advert_interval::advert_invalid) {
	  hm10_test.set_advert_interval(new_interval);
	  debug_log("New advert interval: 0x%01X\n", hm10_test.get_advert_interval());
  }
}

void test_mac_whitelist() {
  debug_log("The current MAC whitelist state: %s\n", hm10_test.get_whitelist_state() ? "On" : "Off");
  hm10_test.set_whitelist_state(true);
  debug_log("The new MAC whitelist state: %s\n", hm10_test.get_whitelist_state() ? "On" : "Off");
  hm10_test.set_whitelist_state(false);

  for (int i = 1; i < 3; i++) {
    debug_log(" The whitelisted MAC #%d: %s\n", i, hm10_test.get_whitelisted_mac(i).mac_address);
  };
  hm10_test.set_whitelisted_mac(1, "AABBCCDDEEFF");
  hm10_test.set_whitelisted_mac(2, "112233445566");

  printf("Whitelisted MACs set!\n");
  for (int i = 1; i < 3; i++) {
	  debug_log("The whitelisted MAC #%d: %s\n", i, hm10_test.get_whitelisted_mac(i).mac_address);
  };
}



void test_slave_latency() {
  debug_log("The current slave latency: %d\n", hm10_test.get_slave_conn_latency());
  hm10_test.set_slave_conn_latency(2);
  debug_log("The new slave latency: %d\n", hm10_test.get_slave_conn_latency());
}

void test_superv_timeout() {
	debug_log("The current supervision timeout: %d\n",
         static_cast<int>(hm10_test.get_conn_superv_timeout()));
  hm10_test.set_conn_superv_timeout(HM10::conn_timeout::conn_timeout_2000ms);
  debug_log("The new supervision timeout: %d\n", static_cast<int>(hm10_test.get_conn_superv_timeout()));
}

void test_conn_updating() {
	debug_log("Current connection updating status: %s\n", hm10_test.get_update_conn() ? "true" : "false");
  hm10_test.set_conn_updating(false);
  debug_log("Current connection updating status: %s\n", hm10_test.get_update_conn() ? "true" : "false");
}

void test_characteristic_value() {
	debug_log("Current characteristic value: 0x%04X\n", hm10_test.get_characteristics_value());
  hm10_test.set_characteristics_value(0xABCD);
  debug_log("New characteristic value: 0x%04X\n", hm10_test.get_characteristics_value());
}
