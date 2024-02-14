#ifndef TEST_MODULES_HPP_
#define TEST_MODULES_HPP_

#include <stdio.h>
#include "hm10.hpp"
#include <cstring>

void test_factory_reset();
void test_baud_rate(HM10::supported_baudrate new_baud = HM10::supported_baudrate::baud_115200);
void test_mac_address(char const* new_mac = "");
void test_advert_interval(HM10::advert_interval new_interval = HM10::advert_interval::advert_546p25ms);
void test_mac_whitelist();
void test_conn_intervals();
void test_slave_latency();
void test_superv_timeout();
void test_conn_updating();
void test_characteristic_value();
void test_notifications();

#endif /* TEST_MODULES_HPP_ */
