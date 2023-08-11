#include "kernelUtil.h"
#include "processes/terminal/interface.h"

extern "C" void _start(BootInfo* bootInfo) {


    KernelInfo kernelInfo = InitializeKernel(bootInfo);
    //GlobalRenderer->Clear(0x272829);
    GlobalRenderer->PrintNL("--- MadOS ---");
    GlobalRenderer->NextLine();
    GlobalRenderer->NextLine();
    InitializeTerminal();
    
    while (true)
    {
        asm ("hlt");
    }
}