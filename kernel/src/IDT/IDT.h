#pragma once
#include <stdint.h>
#include "../BasicRenderer.h"
#include "../cstr.h"
#include "../panic.h"

struct IDTEntry {
	uint16_t    isr_low;      // The lower 16 bits of the ISR's address
	uint16_t    kernel_cs;    // The GDT segment selector that the CPU will load into CS before calling the ISR
	uint8_t	    ist;          // The IST in the TSS that the CPU will load into RSP; set to zero for now
	uint8_t     attributes;   // Type and attributes; see the IDT page
	uint16_t    isr_mid;      // The higher 16 bits of the lower 32 bits of the ISR's address
	uint32_t    isr_high;     // The higher 32 bits of the ISR's address
	uint32_t    reserved;     // Set to zero
} __attribute__((packed));

struct IDTR {
	uint16_t	limit;
	uint64_t	base;
} __attribute__((packed));

__attribute__((aligned(0x10))) 
static IDTEntry idt[256]; // Create an array of IDT entries; aligned for performance

static IDTR idtr;
static bool vectors[256];

const char* Errors[] = {
        "Division Error",
        "Debug",
        "Non-maskable Interrupt",
        "Breakpoint",
        "Overflow",
        "Bound Range Exceeded",
        "Invalid Opcode",
        "Device Not Available",
        "Double Fault",
        "Coprocessor Segment Overrun",
        "Invalid TSS",
        "Segment Not Present",
        "Stack-Segment Fault",
        "General Protection Fault",
        "Page Fault",
        "Reserved",
        "x87 Floating-Point Exception",
        "Alignment Check",
        "Machine Check",
        "SIMD Floating-Point Exception",
        "Virtualization Exception",
        "Control Protection Exception",
        "Reserved",
        "Hypervisor Injection Exception",
        "VMM Communication Exception",
        "Security Exception",
        "Reserved",
        "Triple Fault",
};

extern "C" void PanicExceptionHandler(int64_t isr);
extern "C" void NonErrorExceptionHandler(int64_t isr);
void SetDescriptor(uint8_t vector, void* isr, uint8_t flags);
extern void* isr_stub_table[];
void InitializeIDT();