#include "pit.h"

uint64_t ticks;

void HandleTimer()
{
    ticks++;
}

void sleep(uint16_t interval)
{
    uint16_t target = ticks + interval;
    while (ticks < target) 
    {
        ;
    }
}