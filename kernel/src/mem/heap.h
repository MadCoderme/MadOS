#pragma once
#include <stdint.h>
#include "../paging/paging.h"
#include "../paging/PageFrameAllocator.h"
#include "../BasicRenderer.h"
#include "../cstr.h"
#include "mem.h"

struct HeapBlock
{
    uint32_t size;
    bool free;
    struct HeapBlock* prev;
    struct HeapBlock* next;
};

void InitializeHeap();
void* kmalloc(size_t size);
void kfree(void* ptr);
void Expand(HeapBlock* prev);
void mergeBlock(HeapBlock* block);

