#pragma once
#include <stdint.h>
#include "../../userinput/keyboard.h"
#include "../../mem/heap.h"
#include "../../driver/ahci/ahci.h"
#include "../../system/storage/storage.h"
#include "../../paging/PageFrameAllocator.h"
#include "../../BasicRenderer.h"
#include "../../cstr.h"
#include "../../utils.h"

void InitializeTerminal();
void ExecutePrompt();