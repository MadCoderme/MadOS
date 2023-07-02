#include "IO.h"

void outb(uint8_t val, uint16_t port)
{
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port) : "memory");
}

uint8_t inb(uint16_t port) 
{
    uint8_t val;
    asm volatile("inb %1, %0" : "=a"(val) : "Nd"(port) : "memory");
    return val;
}

void ioWait()
{
    asm volatile("outb %%al, $0x80" :: "a"(0));
}