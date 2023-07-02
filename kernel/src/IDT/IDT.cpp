#include "IDT.h"

extern "C" void PanicExceptionHandler(int64_t isr) {
    asm volatile("cli");
    Panic(Errors[isr]);
    asm volatile ("hlt");
}

extern "C" void NonErrorExceptionHandler(int64_t isr) {
    asm volatile("cli");
    GlobalRenderer->NextLine();
    GlobalRenderer->color = 0x00ff22ff;
    GlobalRenderer->Print("Exception occured: ");
    GlobalRenderer->Print(Errors[isr]);
    asm volatile ("hlt");
}

void SetDescriptor(uint8_t vector, void* isr, uint8_t flags) {
    IDTEntry* descriptor = &idt[vector];
 
    descriptor->isr_low        = (uint64_t)isr & 0xFFFF;
    descriptor->kernel_cs      = 0x08;
    descriptor->ist            = 0;
    descriptor->attributes     = flags;
    descriptor->isr_mid        = ((uint64_t)isr >> 16) & 0xFFFF;
    descriptor->isr_high       = ((uint64_t)isr >> 32) & 0xFFFFFFFF;
    descriptor->reserved       = 0;
}


void InitializeIDT() {
    idtr.base = (uintptr_t)&idt[0];
    idtr.limit = (uint16_t)sizeof(IDTEntry) * 256 - 1;
 
    for (uint8_t vector = 0; vector < 32; vector++) {
        SetDescriptor(vector, isr_stub_table[vector], 0x8E);
        vectors[vector] = true;
    }
 
    asm volatile ("lidt %0" : : "m"(idtr)); // load the new IDT
    asm volatile ("sti"); // set the interrupt flag
}