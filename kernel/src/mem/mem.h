#pragma once
#include "../efiMemory.h"
#include <stdint.h>
#include <stddef.h>

typedef char char8 __attribute__((vector_size(8), aligned(1)));
typedef char char16 __attribute__((vector_size(16), aligned(1)));
typedef char char32 __attribute__((vector_size(32), aligned(1)));
typedef char char32a __attribute__((vector_size(32), aligned(32)));

typedef uint32_t __attribute__((aligned(1))) u32;
typedef uint64_t __attribute__((aligned(1))) u64;

uint64_t GetMemorySize(EFI_MEMORY_DESCRIPTOR* Map, uint64_t MapEntries, uint64_t DescriptorSize);
void *memset(void *s, uint8_t c, size_t n);
