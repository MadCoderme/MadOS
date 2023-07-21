#include "IDT.h"
#include "IST.h"
#include "../IO.h"


__attribute__((aligned(0x10))) 
IDTEntry idt[256];
IDTR idtr;
 
void IdtSetEntry(int n, uint64_t handler)
{
    idt[n].isr_low = (uint16_t)(handler >> 0);
    idt[n].kernel_cs = 0x08;
    idt[n].ist = 0;
    idt[n].attributes = INTERRUPT_GATE;
    idt[n].isr_mid = (uint16_t)(handler >> 16);
    idt[n].isr_high = (uint32_t)(handler >> 32);
    idt[n].reserved = 0;
}


void InitializeIDT() {

    InstallISR();

    idtr.limit = sizeof(idt) - 1;
    idtr.base = (uint64_t)&idt;
    asm volatile("lidtq %0" : : "m" (idtr) : "memory");

}