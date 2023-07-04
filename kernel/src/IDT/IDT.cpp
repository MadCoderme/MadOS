#include "IDT.h"

__attribute__((aligned(0x10))) 
IDTEntry idt[256];
IDTR idtr;
 
void SetDescriptor(int n, uint64_t handler) {
    idt[n].isr_low = (uint16_t)(handler >> 0);
    idt[n].kernel_cs = 0x08;
    idt[n].ist = 0;
    idt[n].attributes = 0x8E;
    idt[n].isr_mid = (uint16_t)(handler >> 16);
    idt[n].isr_high = (uint32_t)(handler >> 32);
    idt[n].reserved = 0;
}


void InitializeIDT() {
    idtr.base = (uint64_t)&idt;
    idtr.limit = 256 * sizeof(IDTEntry) - 1;
    asm volatile("lidtq %0" : "=m"(idtr));
}