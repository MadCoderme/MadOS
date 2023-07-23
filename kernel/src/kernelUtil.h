#include <stddef.h>
#include <cpuid.h>
#include "BasicRenderer.h"
#include "cstr.h"
#include "efiMemory.h"
#include "mem/mem.h"
#include "mem/heap.h"
#include "paging/PageFrameAllocator.h"
#include "paging/PageMapIndexer.h"
#include "paging/PageTableManager.h"
#include "system/acpi/acpi.h"

struct BootInfo {
	FrameBuffer* frameBuffer;
	PSF1_FONT* font;
	EFI_MEMORY_DESCRIPTOR* memoryMap;
	uint64_t mapSize, descriptorSize;
	ACPI::RSDPDescriptor* rsdp;
};

struct KernelInfo {
    PageTableManager* PTM;
};

extern uint64_t _kernelStart;
extern uint64_t _kernelEnd;

void PrepareMemory(BootInfo* bootInfo);
KernelInfo InitializeKernel(BootInfo* bootInfo);