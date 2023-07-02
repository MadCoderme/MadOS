#include "panic.h"

void Panic (const char* message)
{
    GlobalRenderer->Clear(0xffffffff);
    GlobalRenderer->color = 0;
    GlobalRenderer->CursorPosition = {0, 0};
    GlobalRenderer->Print("Damn! We broke :(");
    GlobalRenderer->NextLine();
    GlobalRenderer->NextLine();
    GlobalRenderer->Print(message);
}