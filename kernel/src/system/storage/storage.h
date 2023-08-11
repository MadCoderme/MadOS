#pragma once
#include <stdint.h>
#include "../../driver/ahci/ahci.h"
#include "../../mem/heap.h"
#include "../../BasicRenderer.h"
#include "../../cstr.h"

struct DirectoryTableEntry {
    uint32_t PrevEntry;
    uint32_t NextEntry;
    uint32_t Blocks[128];
};

struct FileMetaData {
    char* Name;
    uint32_t FirstNode;
    uint32_t LastNode;
};

struct FileBlock
{
    uint32_t Metadata;
    uint32_t PrevNode;
    uint32_t NextNode;
    char* buffer;
};

void InitializeDrive();
uint8_t FormatDrive();

class FS {
    public:
    FileMetaData** List();
    char* Open(char* name);
    bool WriteBuffer(char* name, char* wbuf);
};

extern uint8_t MountedDrivePort;


