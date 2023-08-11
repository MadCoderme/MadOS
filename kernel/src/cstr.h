#pragma once
#include <stdint.h>

const char* to_string(int64_t n);
const char* to_string(double value, uint8_t decimalPlaces);
const char* to_string(double value);
const char* to_hstring(uint64_t value);
const char* to_hstring(uint32_t value);
const char* to_hstring(uint16_t value);
const char* to_hstring(uint8_t value);
bool strcmp(char* str, const char* val, uint16_t valLength);
uint64_t atoi(char* str);