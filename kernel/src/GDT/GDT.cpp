#include "GDT.h"


void* Memset(void* start, uint8_t value, size_t len) {
    unsigned char* p = (unsigned char*)start;

    while(len--) {
        *p++ = value;
    }

    return start;
}

__attribute__((aligned(0x1000)))
GDT DefaultGDT = {
    {0, 0, 0, 0x00, 0x00, 0}, 
    {0, 0, 0, 0x9a, 0xa0, 0}, 
    {0, 0, 0, 0x92, 0xa0, 0}, 
    {0, 0, 0, 0xfa, 0xa0, 0},
    {0, 0, 0, 0xf2, 0xa0, 0}, 
    {0, 0, 0, 0x00, 0x00, 0}
};

void InitializeTSS()
{
    TSS_T tss;
    TSS_T* tssPtr = &tss;
    Memset(tssPtr, 0, sizeof(TSS64));
    tss.ioBitmap = sizeof(TSS64);

    uint64_t base = (uint64_t)tssPtr;
    uint32_t limit = base + sizeof(tss) - 1;

    DefaultGDT.TSS = {
        (uint16_t)(limit & 0xffff),
        (uint16_t)(base & 0xffff),
        (uint8_t)((base & 0xff0000) >> 16),
        0xE9,
        0x00,
        (uint8_t)((base & 0xff000000) >> 24)
    };
}