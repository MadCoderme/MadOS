#include "mem.h"

uint64_t GetMemorySize(EFI_MEMORY_DESCRIPTOR* Map, uint64_t MapEntries, uint64_t DescriptorSize) {
    
    static uint64_t memoryMapSize = 0;
    EFI_MEMORY_DESCRIPTOR* desc;
    if(memoryMapSize > 0) return memoryMapSize;

    for (int i = 0; i < MapEntries; i++) {
      desc = (EFI_MEMORY_DESCRIPTOR*)((uint64_t)Map + (i * DescriptorSize));
      memoryMapSize += desc->numPages * 4096;
    }

    return memoryMapSize;
}

// void* memset(void* start, uint8_t value, size_t len) {
//     unsigned char* p = (unsigned char*)start;

//     unsigned char v = value & 0xff;

//     while(len--) {
//         *p++ = v;
//     }

//     return start;
// }

static inline void *small_memset(void *s, int c, size_t n) {
  if (n < 5) {
    if (n == 0)
      return s;
    char *p = (char*)s;
    p[0] = c;
    p[n - 1] = c;
    if (n <= 2)
      return s;
    p[1] = c;
    p[2] = c;
    return s;
  }

  if (n <= 16) {
    uint64_t val8 = ((uint64_t)0x0101010101010101L * ((uint8_t)c));
    if (n >= 8) {
      char *first = (char*)s;
      char *last = (char*)s + n - 8;
      *((u64 *)first) = val8;
      *((u64 *)last) = val8;
      return s;
    }

    uint32_t val4 = val8;
    char *first = (char*)s;
    char *last = (char*)s + n - 4;
    *((u32 *)first) = val4;
    *((u32 *)last) = val4;
    return s;
  }

  char X = c;
  char *p = (char*)s;
  char16 val16 = {X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X};
  char *last = (char*)s + n - 16;
  *((char16 *)last) = val16;
  *((char16 *)p) = val16;
  return s;
}

static inline void *huge_memset(void *s, int c, size_t n) {
  char *p = (char*)s;
  char X = c;
  char32 val32 = {X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
                  X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X};

  // Stamp the first 32byte store.
  *((char32 *)p) = val32;

  char *first_aligned = p + 32 - ((uint64_t)p % 32);
  char *buffer_end = p + n;
  char *last_word = buffer_end - 32;

  // Align the next stores.
  p = first_aligned;

  // Unroll the body of the loop to increase parallelism.
  while (p + (32 * 5) < buffer_end) {
    *((char32a *)p) = val32;
    p += 32;
    *((char32a *)p) = val32;
    p += 32;
    *((char32a *)p) = val32;
    p += 32;
    *((char32a *)p) = val32;
    p += 32;
    *((char32a *)p) = val32;
    p += 32;
  }

// Complete the last few iterations:
#define TRY_STAMP_32_BYTES                                                     \
  if (p < last_word) {                                                         \
    *((char32a *)p) = val32;                                                   \
    p += 32;                                                                   \
  }

  TRY_STAMP_32_BYTES
  TRY_STAMP_32_BYTES
  TRY_STAMP_32_BYTES
  TRY_STAMP_32_BYTES

  // Stamp the last unaligned word.
  *((char32 *)last_word) = val32;
  return s;
}

void *memset(void *s, uint8_t c, size_t n) {
  char *p = (char*)s;
  char X = c;

  if (n < 32) {
    return small_memset(s, c, n);
  }

  if (n > 160) {
    return huge_memset(s, c, n);
  }

  char32 val32 = {X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X,
                  X, X, X, X, X, X, X, X, X, X, X, X, X, X, X, X};

  char *last_word = (char*)s + n - 32;

  // Stamp the 32-byte chunks.
  do {
    *((char32 *)p) = val32;
    p += 32;
  } while (p < last_word);

  // Stamp the last unaligned 32 bytes of the buffer.
  *((char32 *)last_word) = val32;
  return s;
}