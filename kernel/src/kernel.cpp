#include "kernelUtil.h"

extern "C" void _start(BootInfo* bootInfo) {


    KernelInfo kernelInfo = InitializeKernel(bootInfo);
    GlobalRenderer->Print("Kernel Initialized");
    
    // asm("int $0x21");
    while (true);
}