#pragma once
#include <stdint.h>
#include "../efiMemory.h"
#include "../mem/mem.h"
#include "../Bitmap.h"


class PageFrameAllocator {
    public: 
    void ReadEfiMemoryMap(EFI_MEMORY_DESCRIPTOR* Map, size_t MapSize, size_t DescriptorSize);
    Bitmap PageBitmap;
    void* RequestPage();
    void AllocatePage(void* addr);
    void AllocatePages(void* addr, uint64_t pageCount);
    void FreePage(void* addr);
    void FreePages(void* addr, uint64_t pageCount);
    uint64_t GetFreeRAM();
    uint64_t GetUsedRAM();
    uint64_t GetReservedRAM();

    private:
    void InitializeBitmap(size_t bitmapSize, void* bufferAddress);
    void ReservePage(void* addr);
    void ReservePages(void* addr, uint64_t pageCount);
    void UnreservePage(void* addr);
    void UnreservePages(void* addr, uint64_t pageCount);
};

extern PageFrameAllocator GlobalAllocator;