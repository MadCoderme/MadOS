#include "heap.h"

HeapBlock* heapStart;
HeapBlock* heapEnd;
int blockCount = 0;

void InitializeHeap()
{
    HeapBlock* startBlock = (HeapBlock*)GlobalAllocator.RequestPage(); // Allocate the first page frame
    startBlock->size = 0x1000 + sizeof(HeapBlock);
    startBlock->free = true;
    startBlock->prev = NULL;
    startBlock->next = NULL;
    heapStart = startBlock;
    heapEnd = startBlock;
    blockCount = 1; // initialize with a single block of memory (which is a full page)
}

void* kmalloc(size_t size)
{
    if (size % 0x10 > 0) // make sure size is a multiple of 16
    { 
        size -= (size % 0x10);
        size += 0x10;
    }

    if (size == 0) return NULL;

    size += sizeof(HeapBlock); // allocate some extra memory to save the block information

    HeapBlock* block = heapStart; // start from the first memory block

    for (int i = 0; i < blockCount; i++)
    {   
        
        if (block->free) 
        {

            if (block->size < size) 
            {
                if (block->next != NULL) block = block->next;
                continue;  // if the size is not enough, move on the next block that is linked with the current one
            }

            HeapBlock* selectedBlock = block; // found a block that can be worked with
            uint32_t freeSpace = block->size - size; // calculate how much space will be left after the requested amount has been allocated

            // modify the block that will be returned
            selectedBlock->size = size; 
            selectedBlock->free = false;

            // modify the block space that will be left
            block = (HeapBlock*)(block + size);
            block->size = freeSpace;
            block->free = true;

            // link the free block to the one that will be returned
            selectedBlock->next = block;
            block->prev = selectedBlock;
            blockCount++; // increase block count as we just split

            return (void*)(selectedBlock + sizeof(HeapBlock)); // return the address from where bytes can be safely stored without corrupting heap block info
        }

        if (block->next != NULL) block = block->next; // if not found, move on the next block that is linked with the current one
    }

    Expand(block); // no block with a suitable size could be found, so add a new page frame
    mergeBlock(heapEnd); // if block before the newly created page had free space, merge it

    return kmalloc(size);
}

void kfree(void* ptr)
{
    if (ptr == NULL) return;
    HeapBlock* block = (HeapBlock*)ptr - sizeof(HeapBlock); // get to the beginning, including the heap block info
    
    block->free = true;
    mergeBlock(block); // merge the free block
}

void Expand(HeapBlock* prev)
{

    HeapBlock* newBlock = (HeapBlock*) GlobalAllocator.RequestPage(); // create a new page
    newBlock->size = 0x1000 + sizeof(HeapBlock);
    newBlock->free = true;
    newBlock->prev = prev;
    newBlock->next = NULL;
    prev->next = newBlock;
    heapEnd = newBlock; // set it as last block
    blockCount++; // increase counter to include in heap iteration 
}

void mergeBlock(HeapBlock* block)
{
    if (block->next && block->next->free)
    {
        block->size += block->next->size;
        block->next = block->next->next;
        blockCount--;
    }

    if (block->prev && block->prev->free)
    {
        block->prev->size += block->size;
        block->prev->next = block->next;
        blockCount--;
    }

}

