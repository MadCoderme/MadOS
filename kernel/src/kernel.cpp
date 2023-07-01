#include "kernelUtil.h"

extern "C" void _start(BootInfo* bootInfo) {

    GlobalRenderer.Initialize(bootInfo->frameBuffer, bootInfo->font);
    KernelInfo kernelInfo = InitializeKernel(bootInfo);

    GlobalRenderer.Print("Kernel Initialized");

    
    while (true);
    return;
}