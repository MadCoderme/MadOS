#pragma once
#include <stdint.h>

class PageMapIndexer {
    public:
    PageMapIndexer(uint64_t virtualAddress);
    uint64_t PDP_i, PD_i, PT_i, P_i;    
};