#include "kernelUtil.h"
#include "GDT/GDT.h"
#include "interrupts/IDT.h"
#include "interrupts/pic.h"
#include "system/pci/pci.h"
#include "system/storage/storage.h"


KernelInfo kernelInfo;

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


    GlobalPTM.PML4 = PML4;

    for (uint64_t i = 0; i < memSize; i+=0x1000)
    {
        GlobalPTM.MapMemory((void*)i, (void*)i);
    }


    uint64_t frameBufferBase = (uint64_t)bootInfo->frameBuffer->BaseAddress;
    uint64_t frameBufferSize = bootInfo->frameBuffer->BufferSize + 0x1000;
    GlobalAllocator.AllocatePages((void*)frameBufferBase, frameBufferSize / 0x1000 + 1);
    

    for (uint64_t i = frameBufferBase; i < frameBufferBase + frameBufferSize; i+=4096)
    {
        GlobalPTM.MapMemory((void*)i, (void*)i);
    }
    
    enablePaging(PML4);


    kernelInfo.PTM = &GlobalPTM;
}

void SetupGDT()
{
    GDTDescriptor gdtDescriptor;
    // InitializeTSS();
    gdtDescriptor.Size = sizeof(GDT) - 1;
    gdtDescriptor.Offset = (uint64_t)&DefaultGDT;
    setGDT(&gdtDescriptor);
}

void SetupInterrupts()
{   
    InitializeIDT();
    asm volatile("sti");
}

void SetupACPI(ACPI::RSDPDescriptor* rsdp)
{
    ACPI::ACPIMCFGHeader* mcfg = (ACPI::ACPIMCFGHeader*)ACPI::FindTable(rsdp, (char*)"MCFG");
    PCIExpress::EnumeratePIC(mcfg);
}


BasicRenderer renderer = BasicRenderer(NULL, NULL);

KernelInfo InitializeKernel(BootInfo* bootInfo)
{
    renderer = BasicRenderer(bootInfo->frameBuffer, bootInfo->font);
    GlobalRenderer = &renderer;

    PrepareMemory(bootInfo);
    SetupGDT();
    SetupInterrupts();
    SetupACPI(bootInfo->rsdp);
    InitializeHeap();
    InitializeDrive();
    

    // asm volatile("mov %ax, 0x21");
    // asm volatile("ltr %ax");
    

    //memset(bootInfo->frameBuffer->BaseAddress, 0, bootInfo->frameBuffer->BufferSize);

    return kernelInfo;
}

