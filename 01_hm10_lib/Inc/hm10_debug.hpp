#pragma once
#include <cstdio>

#define HM10_GEN_DEBUG
#define HM10_DEBUG_LEVEL2

#ifdef HM10_GEN_DEBUG
#define debug_log(format, ...) std::printf("[DEBUG] <%s:%d>: " format "\n", __func__, __LINE__, ## __VA_ARGS__)
#else
#define debug_log(...)
#endif

#ifdef HM10_DEBUG_LEVEL2
#define debug_log_level2(format, ...) std::printf("[DEBUG LEVEL2]<%s:%d>: " format "\n", __func__, __LINE__, ## __VA_ARGS__)
#else
#define debug_log_level2(...)
#endif


void log_error(char* p);
void log_info(char* p);
