#include "IDT.h"
#include "pic.h"
#include "../IO.h"

__attribute__((aligned(0x10))) 
IDTEntry idt[256];
IDTR idtr;

static const char *s_exceptionDesc[] =
{
    "#DE: Divide Error",
    "#DB: Debug Exception",
    " — : NMI Interrupt",
    "#BP: Breakpoint",
    "#OF: Overflow",
    "#BR: BOUND Range Exceeded",
    "#UD: Invalid Opcode (Undefined Opcode)",
    "#NM: Device Not Available (No Math Coprocessor)",
    "#DF: Double Fault",
    "— : Coprocessor Segment Overrun (reserved)",
    "#TS: Invalid TSS",
    "#NP: Segment Not Present",
    "#SS: Stack-Segment Fault",
    "#GP: General Protection",
    "#PF: Page Fault",
    "— : (Intel reserved. Do not use.)",
    "#MF: x87 FPU Floating-Point Error (Math Fault)",
    "#AC: Alignment Check",
    "#MC: Machine Check",
    "#XM: SIMD Floating-Point Exception",
    "#VE: Virtualization Exception",
    "#CP: Control Protection Exception",
    "— : Intel reserved. Do not use.",
    "— : Intel reserved. Do not use.",
    "— : Intel reserved. Do not use.",
    "— : Intel reserved. Do not use.",
    "— : Intel reserved. Do not use.",
    "— : Intel reserved. Do not use.",
    "— : Intel reserved. Do not use.",
    "— : Intel reserved. Do not use.",
    "— : Intel reserved. Do not use.",
    "— : Intel reserved. Do not use."
};
 
static void IdtSetEntry(uint8_t index, uint64_t base, uint16_t selector, uint16_t type)
{
    IDTEntry *entry = (IDTEntry*)&idt + index;

    entry->isr_low = (uint16_t)base;
    entry->kernel_cs = selector;
    entry->attributes = type;
    entry->ist = 0;
    entry->isr_mid = (uint16_t)(base >> 16);
    entry->isr_high = (uint32_t)(base >> 32);
    entry->reserved = 0;
}

void IdtSetHandler(uint8_t index, uint16_t type, void (*handler)())
{
    if (handler)
    {
        IdtSetEntry(index, (uint64_t)handler, 0x08, type);
    }
    else
    {
        IdtSetEntry(index, 0, 0, 0);
    }
}


void InitializeIDT() {
    // Build initial IDT
    for (int i = 0; i < 20; ++i)
    {
        IdtSetHandler(i, INTERRUPT_GATE, exception_handlers[i]);
    }

    for (int i = 20; i < 32; ++i)
    {
        IdtSetHandler(i, INTERRUPT_GATE, default_exception_handler);
    }


    RemapPIC();

    for (int i = 32; i < 48; ++i)
    {
        IdtSetHandler(i, INTERRUPT_GATE, exception_handlers[i - 12]);
    }

    for (int i = 48; i < 256; ++i)
    {
        IdtSetHandler(i, INTERRUPT_GATE, default_interrupt_handler);
    }

    idtr.limit = sizeof(idt) - 1;
    idtr.base = (uint64_t)&idt;
    asm volatile("lidtq %0" : : "m" (idtr) : "memory");

}


extern "C" void ExceptionDump(int r)
{
    const char *desc = "Unknown";
    if (r < 20)
    {
        desc = s_exceptionDesc[r];

        GlobalRenderer->color = 0xff00ffff;
        GlobalRenderer->NextLine();
        GlobalRenderer->Print("Exception: ");
        GlobalRenderer->Print(desc);
        GlobalRenderer->NextLine();
        // GlobalRenderer->Print("  rax=");
        // GlobalRenderer->Print(to_string((int64_t)r.rax));
        // GlobalRenderer->NextLine();
        // GlobalRenderer->Print("  rbx=");
        // GlobalRenderer->Print(to_string((int64_t)r.rbx));
        // GlobalRenderer->NextLine();
        // GlobalRenderer->Print("  rcx=");
        // GlobalRenderer->Print(to_string((int64_t)r.rcx));
        // GlobalRenderer->NextLine();
        // GlobalRenderer->Print("  rdx=");
        // GlobalRenderer->Print(to_string((int64_t)r.rdx));
        // GlobalRenderer->NextLine();
        // GlobalRenderer->Print("  rsi=");
        // GlobalRenderer->Print(to_string((int64_t)r.rsi));
        // GlobalRenderer->NextLine();
        // GlobalRenderer->Print("  rdi=");
        // GlobalRenderer->Print(to_string((int64_t)r.rdi));
        // GlobalRenderer->NextLine();
        // GlobalRenderer->Print("  rip=");
        // GlobalRenderer->Print(to_string((int64_t)r.rip));
        // GlobalRenderer->NextLine();
        // GlobalRenderer->Print("  rsp=");
        // GlobalRenderer->Print(to_string((int64_t)r.rsp));
        // GlobalRenderer->NextLine();
        // GlobalRenderer->Print("  cs=");
        // GlobalRenderer->Print(to_string((int64_t)r.cs));
        // GlobalRenderer->NextLine();
        // GlobalRenderer->Print("  ss=");
        // GlobalRenderer->Print(to_string((int64_t)r.ss));
        // GlobalRenderer->NextLine();

        asm("hlt");
    }
    else 
    {
        
        if (r == 33)
            GlobalRenderer->Print(to_string((int64_t)r));
    }

    
}