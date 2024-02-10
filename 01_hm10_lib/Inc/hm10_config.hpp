#ifndef HM10_CONFIG_HPP_
#define HM10_CONFIG_HPP_

#include <cstdint>

namespace HM10
{
  enum class supported_baudrate: std::uint8_t       // HM-10 datasheet page: 17 [14. Query/Set baud rate]
  {
	baud_9600        = 0,
	baud_19200       = 1,
	baud_38400       = 2,
	baud_57600       = 3,
	baud_115200      = 4,
	baud_4800        = 5,
	baud_2400        = 6,
	baud_1200        = 7,
	baud_230400      = 8,
	baud_invalid     = 9,
  };

  constexpr std::uint32_t baudrate_values[] = {      // `supported_baudrate` lookup table
	9600, 19200, 38400, 57600, 115200, 4800, 2400, 1200, 230400, 0
  };

  enum class advert_interval: std::uint8_t          // HM-10 datasheet page: 11 [3. Query/Set Advertising interval]
  {
	advert_100ms     = 0,
	advert_152p5ms   = 1,
	advert_211p25ms  = 2,
	advert_318p75ms  = 3,
	advert_417p5ms   = 4,
	advert_546p25ms  = 5,
	advert_760ms     = 6,
	advert_852p5ms   = 7,
	advert_1022p5ms  = 8,
	advert_1285ms    = 9,
	advert_2000ms    = 10,
	advert_3000ms    = 11,
	advert_4000ms    = 12,
	advert_5000ms    = 13,
	advert_6000ms    = 14,
	advert_7000ms    = 15,
	advert_invalid   = 255,
  };

  enum class advert_type: std::uint8_t              // HM-10 datasheet page: 11 [4. Query/Set Advertising type]
  {
	advert_type_all              = 0,
	advert_type_conn_last_device = 1,
	advert_type_scan_resp        = 2,
	advert_type_advert           = 3,
	advert_type_invalid          = 255,
  };

  enum class conn_interval: std::uint8_t            // HM-10 datasheet page: 17 [15. Query/Set Minimum Link Layer connection interval]
  {
	conn_7p5ms       = 0,
	conn_10ms        = 1,
	conn_15ms        = 2,
	conn_20ms        = 3,
	conn_25ms        = 4,
	conn_30ms        = 5,
	conn_35ms        = 6,
	conn_40ms        = 7,
	conn_45ms        = 8,
	conn_4000ms      = 9,
	conn_invalid     = 255,
  };

  enum class conn_timeout: std::uint8_t             // HM-10 datasheet page: 18 [18. Query/Set connection supervision timeout]
  {
	conn_timeout_100ms           = 0,
	conn_timeout_1000ms          = 1,
	conn_timeout_2000ms          = 2,
	conn_timeout_3000ms          = 3,
	conn_timeout_4000ms          = 4,
	conn_timeout_5000ms          = 5,
	conn_timeout_6000ms          = 6,
	conn_timeout_invalid         = 255,
  };

  enum class work_mode: std::uint8_t                // HM-10 datasheet page: 9 [System work mode]
  {
	mode_tx                      = 0,
	mode_pio_acquisition	     = 1,
	mode_remote_control          = 2,
	mode_invalid                 = 255,
  };

  enum class module_power: std::uint8_t             // HM-10 datasheet page: 32 [56. Query/Set Module Power]
  {
	power_minus23dbm = 0,
	power_minus6dbm  = 1,
	power_0dbm       = 2,
	power_6dbm       = 3,
	power_invalid    = 255,
  };

  enum class output_power: std::uint8_t             // HM-10 datasheet page: 29 [51. Query/Set output driver power]
  {
	output_power_normal         = 0,
	output_power_max            = 1,
	output_power_invalid        = 255,
  };

  enum class role: std::uint8_t                     // HM-10 datasheet page: 33 [61. Query/Set Master and Slaver Role]
  {
	role_peripheral  = 0,
	role_central     = 1,
	role_invalid     = 255,
  };

  enum class bond_mode: std::uint8_t                // HM-10 datasheet page: 37 [76. Query/Set Module Bond Mode]
  {
	mode_no_pin      = 0,
	mode_auth_no_pin = 1,
	mode_auth_pin    = 2,
	mode_auth_bond   = 3,
	mode_invalid     = 255,
  };

  struct device_name
  {
	char name[15];
  };

  struct device_version
  {
	char version[15];
  };

  struct mac_address                                // HM-10 datasheet page: 8 [System advert packet]
  {
	char mac_address[15];
  };

}

#endif /* HM10_CONFIG_HPP_ */
