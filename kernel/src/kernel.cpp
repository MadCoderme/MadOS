#include "kernelUtil.h"
#include "IO.h"

extern "C" void _start(BootInfo* bootInfo) {


    KernelInfo kernelInfo = InitializeKernel(bootInfo);
    ioWait();
    GlobalRenderer->Print("Kernel Initialized");
    
    while (true);
}