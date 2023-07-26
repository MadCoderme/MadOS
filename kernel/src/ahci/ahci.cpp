#include "ahci.h"
#include "../BasicRenderer.h"
#include "../cstr.h"

void ProbePort(HBAMem* abar)
{
    GlobalRenderer->Print(to_string((int64_t)abar->vs));
}