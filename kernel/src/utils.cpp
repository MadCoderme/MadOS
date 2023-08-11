#include "utils.h"

void PrintMemorySize(uint64_t sizeInBytes)
{

    if (sizeInBytes > 1024 * 1024 * 1024)
    {
        GlobalRenderer->Print(to_string((double)(sizeInBytes / (1024 * 1024 * 1024))));
        GlobalRenderer->Print(" GB");
    }
    else 
    if (sizeInBytes > 1024 * 1024)
    {
        GlobalRenderer->Print(to_string((double)(sizeInBytes / (1024 * 1024))));
        GlobalRenderer->Print(" MB");
    }
    else
    if (sizeInBytes > 1024)
    {
        GlobalRenderer->Print(to_string((double)(sizeInBytes / 1024)));
        GlobalRenderer->Print(" KB");
    }
    else 
    {
        GlobalRenderer->Print(to_string((double)sizeInBytes));
        GlobalRenderer->Print(" B");
    }
}