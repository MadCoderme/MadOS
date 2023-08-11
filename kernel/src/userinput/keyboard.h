#pragma once
#include <stdint.h>
#include "../IO.h"
#include "../BasicRenderer.h"

void EnableKeyboard(bool willEnable);
void ReceiveInput(char* c);
void TranslateScancode(uint8_t byte);
void HandleKeyboard();

void EnterHandler();