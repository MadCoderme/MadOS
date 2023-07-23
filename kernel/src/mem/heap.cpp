#include "heap.h"

HeapBlock* heapStart;
void* heapEnd;
HeapBlock* lastBlock;
int blockCount = 0;

void InitializeHeap()
{
    HeapBlock* startBlock = (HeapBlock*)GlobalAllocator.RequestPage();
    startBlock->size = 0x1000;
    startBlock->free = true;
    startBlock->prev = NULL;
    startBlock->next = NULL;
    heapStart = startBlock;
    blockCount = 1;
}

void* kmalloc(size_t size)
{
    if (size % 0x10 > 0)
    { 
        size -= (size % 0x10);
        size += 0x10;
    }

    if (size == 0) return NULL;

    HeapBlock* block = heapStart;
    for (int i = 0; i < blockCount; i++)
    {   
        if (block->free) 
        {
            if (block->size < size) 
            {
                block = block->next;
                continue;
            }

            HeapBlock* selectedBlock = block;
            uint64_t freeSpace = block->size - size;
            selectedBlock->size = size;
            selectedBlock->free = false;

            block = (HeapBlock*)(block + size);
            block->size = freeSpace;
            block->free = true;
            selectedBlock->next = block;
            block->prev = selectedBlock;
            blockCount++;

            return (void*)selectedBlock;
        }


        block = block->next;
    }

    Expand(block);
    return kmalloc(size);
}

void kfree(void* ptr)
{
    HeapBlock* block = (HeapBlock*)ptr;
    block->free = true;
    mergeBlock(block);
}

void Expand(HeapBlock* prev)
{
    HeapBlock* newBlock = (HeapBlock*) GlobalAllocator.RequestPage();
    newBlock->size = 0x1000;
    newBlock->free = true;
    newBlock->prev = prev;
    newBlock->next = NULL;
    prev->next = newBlock;
    blockCount++;
}

void mergeBlock(HeapBlock* block)
{
    if (block->prev->free)
    {
        block->size += block->prev->size;
        block->prev = block->prev->prev;
        blockCount--;
    }

    if (block->next->free)
    {
        block->size += block->next->size;
        block->next = block->next->next;
        blockCount--;
    }
}

