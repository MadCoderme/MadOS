#pragma once
#include <stdint.h>

enum PT_Flags {
    Present = 0,
    ReadWrite = 1,
    UserSuper = 2,
    WriteThrough = 3,
    CacheDisabled = 4,
    Accessed = 5,
    LargerPages = 7,
    Custom0 = 9,
    Custom1 = 10,
    Custom2 = 11,
    NX = 63
};

struct PageDirectoryEntry {
    uint64_t Value;
    void setFlag(PT_Flags flag, bool value);
    bool getFlag(PT_Flags flag);
    void setAddress(uint64_t addr);
    uint64_t getAddress();
};

struct PageTable {
    PageDirectoryEntry entries [512];
}__attribute__((aligned(0x1000)));

extern "C" void enablePaging(PageTable* PML4);

