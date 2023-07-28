#pragma once
#include "paging.h"
#include "PageMapIndexer.h"
#include "PageFrameAllocator.h"

class PageTableManager {
    public:
    // PageTableManager(PageTable* PML4Address);
    PageTable* PML4;
    void MapMemory(void* virtualMemory, void* physicalMemory);
};

extern PageTableManager GlobalPTM;