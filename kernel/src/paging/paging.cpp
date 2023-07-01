#include "paging.h"

void PageDirectoryEntry::setFlag(PT_Flags flag, bool value) 
{
    uint64_t bitSelector = (uint64_t)1 << flag;
    Value &= ~bitSelector;
    if (value) {
        Value |= bitSelector;
    }
}

bool PageDirectoryEntry::getFlag(PT_Flags flag)
{
    uint64_t bitSelector = (uint64_t)1 << flag;
    if (Value & bitSelector > 0) return true;
    return false;   
}

void PageDirectoryEntry::setAddress(uint64_t addr) 
{
    addr &= 0x000000ffffffffff;
    Value &= 0xfff0000000000fff;
    Value |= (addr << 12);
}

uint64_t PageDirectoryEntry::getAddress() 
{
    return (Value & 0x000ffffffffff000) >> 12;
}