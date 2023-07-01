#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

class Bitmap {
    public:
    size_t size;
    uint8_t* Buffer;
    bool operator[](uint64_t index);
    bool Set(uint64_t index, bool value);
};