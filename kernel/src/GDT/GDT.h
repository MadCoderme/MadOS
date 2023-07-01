#pragma once
#include <stdint.h>
#include <memory.h>

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
	uint64_t rsp0;
	uint64_t rsp1;
	uint64_t rsp2;
	uint64_t reserved1;
	uint64_t ist1;
	uint64_t ist2;
	uint64_t ist3;
	uint64_t ist4;
	uint64_t ist5;
	uint64_t ist6;
	uint64_t ist7;
	uint64_t reserved2;
	uint16_t reserved3;
	uint8_t ioBitmap[8192];
} __attribute__ (( packed ));


extern GDT DefaultGDT;
extern "C" void setGDT(GDTDescriptor* gdtDescriptor);