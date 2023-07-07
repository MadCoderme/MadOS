#include "kernelUtil.h"
#include "memory.h"
#include "GDT/GDT.h"
#include "interrupts/IDT.h"
#include "interrupts/pic.h"

KernelInfo kernelInfo;
PageTableManager ptm = NULL;


void PrepareMemory(BootInfo* bootInfo) 
{
    uint64_t entries = bootInfo->mapSize / bootInfo->descriptorSize;

    GlobalAllocator = PageFrameAllocator();
    GlobalAllocator.ReadEfiMemoryMap(bootInfo->memoryMap, bootInfo->mapSize, bootInfo->descriptorSize);
    uint64_t memSize = GetMemorySize(bootInfo->memoryMap, entries, bootInfo->descriptorSize);

    uint64_t kernelSize = (uint64_t)&_kernelEnd - (uint64_t)&_kernelStart;
    uint64_t pages = kernelSize / 4096 + 1;
    GlobalAllocator.AllocatePages(&_kernelStart, pages);

    PageTable* PML4 = (PageTable*)GlobalAllocator.RequestPage();
    memset(PML4, 0, 0x1000);

    ptm = PageTableManager(PML4);

    for (uint64_t i = 0; i < memSize; i+=0x1000)
    {
        ptm.MapMemory((void*)i, (void*)i);
    }
    
    uint64_t frameBufferBase = (uint64_t)bootInfo->frameBuffer->BaseAddress;
    uint64_t frameBufferSize = bootInfo->frameBuffer->BufferSize + 0x1000;
    GlobalAllocator.AllocatePages((void*)frameBufferBase, frameBufferSize / 0x1000 + 1);

    for (uint64_t i = frameBufferBase; i < frameBufferBase + frameBufferSize; i+=4096)
    {
        ptm.MapMemory((void*)i, (void*)i);
    }
    
    // asm volatile ("mov %%cr4, %%eax; bts $5, %%eax; mov %%eax, %%cr4" ::: "eax");	 
    asm volatile ("mov %0, %%cr3" :: "r" (PML4));
    // asm volatile ("mov %%cr0, %%eax; or $0x80000000, %%eax; mov %%eax, %%cr0;" ::: "eax");


    kernelInfo.PTM = &ptm;
}


void PrepareInterrupts()
{   
    InitializeIDT();
}


BasicRenderer renderer = BasicRenderer(NULL, NULL);

KernelInfo InitializeKernel(BootInfo* bootInfo)
{
    renderer = BasicRenderer(bootInfo->frameBuffer, bootInfo->font);
    GlobalRenderer = &renderer;

    PrepareMemory(bootInfo);

    GDTDescriptor gdtDescriptor;
    InitializeTSS();
    gdtDescriptor.Size = sizeof(GDT) - 1;
    gdtDescriptor.Offset = (uint64_t)&DefaultGDT;
    setGDT(&gdtDescriptor);

    PrepareInterrupts();


    asm volatile("mov %ax, 0x21");
    // asm volatile("ltr %ax");
    

    // memset(bootInfo->frameBuffer->BaseAddress, 0, bootInfo->frameBuffer->BufferSize);

    return kernelInfo;
}

