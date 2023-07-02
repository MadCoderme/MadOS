#pragma once
#include <stdint.h>
#include <stddef.h>
#include "../memory.h"

struct GDTDescriptor {
    uint16_t Size;
    uint64_t Offset;
} __attribute__((packed));

struct GDTEntry
{
    uint16_t LimitLow;
    uint16_t BaseLow;
    uint8_t BaseMiddle;
    uint8_t Access;
    uint8_t Granularity;
    uint8_t BaseHigh;
} __attribute__((packed));

struct GDT {
    GDTEntry Null; //0x00
    GDTEntry KernelCode; //0x08
    GDTEntry KernelData; //0x10
    GDTEntry UserCode;
    GDTEntry UserData;
    GDTEntry TSS;
} __attribute__((packed)) 
__attribute((aligned(0x1000)));

struct TSS64 {
	uint32_t reserved0;
	uint64_t rsp[3];
	uint64_t reserved1;
	uint64_t ist[7];
	uint64_t reserved2;
	uint16_t reserved3;
	uint16_t ioBitmap;
} __attribute__ (( packed ));
typedef struct TSS64 TSS_T;

void InitializeTSS();

extern GDT DefaultGDT;
extern "C" void setGDT(GDTDescriptor* gdtDescriptor);