#include <stddef.h>
#include "BasicRenderer.h"
#include "cstr.h"
#include "efiMemory.h"
#include "memory.h"

#include "paging/PageFrameAllocator.h"
#include "paging/PageMapIndexer.h"
#include "paging/PageTableManager.h"

struct BootInfo {
	FrameBuffer* frameBuffer;
	PSF1_FONT* font;
	EFI_MEMORY_DESCRIPTOR* memoryMap;
	uint64_t mapSize, descriptorSize;
};

struct KernelInfo {
    PageTableManager* PTM;
};

extern uint64_t _kernelStart;
extern uint64_t _kernelEnd;

void PrepareMemory(BootInfo* bootInfo);
KernelInfo InitializeKernel(BootInfo* bootInfo);