#include "GDT.h"


GDT DefaultGDT = {
    {0, 0, 0, 0x00, 0x00, 0}, 
    {0, 0, 0, 0x9a, 0xa0, 0}, 
    {0, 0, 0, 0x92, 0xa0, 0}, 
    {0, 0, 0, 0xfa, 0xa0, 0}, 
    {0, 0, 0, 0xf2, 0xa0, 0}, 
    {}
};

