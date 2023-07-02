#include "kernelUtil.h"

extern "C" void _start(BootInfo* bootInfo) {


    KernelInfo kernelInfo = InitializeKernel(bootInfo);

    GlobalRenderer->Print("Kernel Initialized");


    
    while (true);
    return;
}