#include "cstr.h"
#include <stdint.h>

const char* to_string(int64_t n){

    static char buffer[50];
    int i = 0;

    int isNeg = n < 0;

    unsigned int n1 = isNeg ? -n : n;

    while(n1 != 0)
    {
        buffer[i++] = n1 % 10 + '0';
        n1 = n1/10;
    }

    if(isNeg)
        buffer[i++] = '-';

    buffer[i] = '\0';

    for(int t = 0; t < i/2; t++)
    {
        buffer[t] ^= buffer[i-t-1];
        buffer[i-t-1] ^= buffer[t];
        buffer[t] ^= buffer[i-t-1];
    }

    if(n == 0)
    {
        buffer[0] = '0';
        buffer[1] = '\0';
    }  

    return buffer;

}

char doubleToString[128];
const char* to_string(double value, uint8_t decimalPlaces) {
    char* intPtr = (char*)to_string((int64_t)value);
    char* doublePtr = doubleToString;

    if(value < 0) value = -value;

    while(*intPtr != 0) {
        *doublePtr = *intPtr;
        intPtr++;
        doublePtr++;
    }

    *doublePtr = '.';
    doublePtr++;

    double newValue = value - (int)value;

    for (uint8_t i = 0; i < decimalPlaces; i++) {
        newValue *= 10;
        *doublePtr = (int)newValue + '0';
        newValue -= (int)newValue;
        doublePtr++;
    }

    *doublePtr = 0;
    return doubleToString;
}

const char* to_string(double value) {
    return to_string(value, 2);
}

char hexToString[128];
const char* to_hstring(uint64_t value) {
    uint64_t* valuePtr = &value;
    uint8_t* ptr;
    uint8_t tmp;
    uint8_t size = 15;
    for (uint8_t i = 0; i < size; i++) {
        ptr = ((uint8_t*)valuePtr + i);
        tmp = ((*ptr & 0xF0) >> 4);
        hexToString[size - (i * 2 + 1)] = tmp + (tmp > 9 ? 55 : '0');
        tmp = ((*ptr & 0x0F));
        hexToString[size - (i * 2)] = tmp + (tmp > 9 ? 55 : '0');
    }

    hexToString[size + 1] = 0;
    return hexToString;
}

char hexToString32[128];
const char* to_hstring(uint32_t value) {
    uint32_t* valuePtr = &value;
    uint8_t* ptr;
    uint8_t tmp;
    uint8_t size = 3;
    for (uint8_t i = 0; i < size; i++) {
        ptr = ((uint8_t*)valuePtr + i);
        tmp = ((*ptr & 0xF0) >> 4);
        hexToString32[size - (i * 2 + 1)] = tmp + (tmp > 9 ? 55 : '0');
        tmp = ((*ptr & 0x0F));
        hexToString32[size - (i * 2)] = tmp + (tmp > 9 ? 55 : '0');
    }

    hexToString[size + 1] = 0;
    return hexToString32;
}

char hexToString16[128];
const char* to_hstring(uint16_t value) {
    uint16_t* valuePtr = &value;
    uint8_t* ptr;
    uint8_t tmp;
    uint8_t size = 3;
    for (uint8_t i = 0; i < size; i++) {
        ptr = ((uint8_t*)valuePtr + i);
        tmp = ((*ptr & 0xF0) >> 4);
        hexToString16[size - (i * 2 + 1)] = tmp + (tmp > 9 ? 55 : '0');
        tmp = ((*ptr & 0x0F));
        hexToString16[size - (i * 2)] = tmp + (tmp > 9 ? 55 : '0');
    }

    hexToString[size + 1] = 0;
    return hexToString16;
}

char hexToString8[128];
const char* to_hstring(uint8_t value) {
    uint8_t* valuePtr = &value;
    uint8_t* ptr;
    uint8_t tmp;
    uint8_t size = 1 * 2 - 1;
    for (uint8_t i = 0; i < size; i++) {
        ptr = ((uint8_t*)valuePtr + i);
        tmp = ((*ptr & 0xF0) >> 4);
        hexToString8[size - (i * 2 + 1)] = tmp + (tmp > 9 ? 55 : '0');
        tmp = ((*ptr & 0x0F));
        hexToString8[size - (i * 2)] = tmp + (tmp > 9 ? 55 : '0');
    }

    hexToString8[size + 1] = 0;
    return hexToString8;
}