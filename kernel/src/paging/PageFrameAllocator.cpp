#include "PageFrameAllocator.h"

uint64_t allocatedMemory;
uint64_t reservedMemory;
uint64_t freeMemory;
bool isInitialized = false;
PageFrameAllocator GlobalAllocator;

void PageFrameAllocator::ReadEfiMemoryMap(EFI_MEMORY_DESCRIPTOR* Map, size_t MapSize, size_t DescriptorSize) {
    if(isInitialized) return;

    isInitialized = true;

    uint64_t entries = MapSize / DescriptorSize;

    void* largestFreeMemSeg = NULL;
    size_t largestFreeMemSize = 0;

    for (int i = 0; i < entries; i++) {
        EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)((uint64_t)Map + (i * DescriptorSize));
        if(desc->type == 7) {
            if(desc->numPages * 4096 > largestFreeMemSize) {
                largestFreeMemSeg = desc->physAddr;
                largestFreeMemSize = desc->numPages * 4096;
            }
        }
    }

    uint64_t memorySize = GetMemorySize(Map, entries, DescriptorSize);
    freeMemory = memorySize;
    
    uint64_t bitmapSize = memorySize / 4096 / 8 + 1;
    InitializeBitmap(bitmapSize, largestFreeMemSeg);

    AllocatePages(&PageBitmap, PageBitmap.size / 4096 + 1);

    for (int i = 0; i < entries; i++) {
        EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)((uint64_t)Map + (i * DescriptorSize));
        if(desc->type != 7) {
            ReservePages(desc->physAddr, desc->numPages);
        }
    }
}

void PageFrameAllocator::InitializeBitmap(size_t bitmapSize, void* bufferAddress) {
    PageBitmap.size = bitmapSize;
    PageBitmap.Buffer = (uint8_t*)bufferAddress;
    for (int i = 0; i < bitmapSize; i++) {
        *(uint8_t*)(PageBitmap.Buffer + i) = 0;
    }
}

uint64_t bitmapIndex = 0;
void* PageFrameAllocator::RequestPage() {
    for(; bitmapIndex < PageBitmap.size * 8; bitmapIndex++) {
        if(PageBitmap[bitmapIndex]) continue;
        AllocatePage((void*)(bitmapIndex * 4096));
        return (void*)(bitmapIndex * 4096);
    }
    return NULL;
}

void PageFrameAllocator::AllocatePage(void* addr) {
    uint64_t i = (uint64_t)addr / 4096;
    if(PageBitmap[i]) return;
    if (PageBitmap.Set(i, true)) {
        allocatedMemory += 4096;
        freeMemory -= 4096;
    }
}

void PageFrameAllocator::AllocatePages(void* addr, uint64_t pageCount) {
    uint64_t startIndex = (uint64_t)addr / 4096;
    for(uint64_t i = startIndex; i < startIndex + pageCount; i++) {
        AllocatePage((void*)(i * 4096));
    }
}

void PageFrameAllocator::FreePage(void* addr) {
    uint64_t i = (uint64_t)addr / 4096;
    if(!PageBitmap[i]) return;
    if (PageBitmap.Set(i, false)) {
        allocatedMemory -= 4096;
        freeMemory += 4096;
        if (bitmapIndex > i) bitmapIndex = i;
    }
}

void PageFrameAllocator::FreePages(void* addr, uint64_t pageCount) {
    uint64_t startIndex = (uint64_t)addr / 4096;
    for(uint64_t i = startIndex; i < startIndex + pageCount; i++) {
        FreePage((void*)(i * 4096));
    }
}

void PageFrameAllocator::ReservePage(void* addr) {
    uint64_t i = (uint64_t)addr / 4096;
    if(PageBitmap[i]) return;
    if (PageBitmap.Set(i, true)) {
        reservedMemory += 4096;
        freeMemory -= 4096;
    }
}

void PageFrameAllocator::ReservePages(void* addr, uint64_t pageCount) {
    uint64_t startIndex = (uint64_t)addr / 4096;
    for(uint64_t i = startIndex; i < startIndex + pageCount; i++) {
        ReservePage((void*)(i * 4096));
    }
}

void PageFrameAllocator::UnreservePage(void* addr) {
    uint64_t i = (uint64_t)addr / 4096;
    if(!PageBitmap[i]) return;
    if (PageBitmap.Set(i, false)) {
        reservedMemory -= 4096;
        freeMemory += 4096;
        if (bitmapIndex > i) bitmapIndex = i;
    }
}

void PageFrameAllocator::UnreservePages(void* addr, uint64_t pageCount) {
    uint64_t startIndex = (uint64_t)addr / 4096;
    for(uint64_t i = startIndex; i < startIndex + pageCount; i++) {
        UnreservePage((void*)(i * 4096));
    }
}

uint64_t PageFrameAllocator::GetFreeRAM(){
    return freeMemory;
}
uint64_t PageFrameAllocator::GetUsedRAM(){
    return allocatedMemory;
}
uint64_t PageFrameAllocator::GetReservedRAM(){
    return reservedMemory;
}
