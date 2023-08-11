#include "storage.h"

DirectoryTableEntry* DirectoryTable;
bool* Sectors;
uint32_t SectorCount;
uint8_t MountedDrivePort = 1;

void InitializeDrive()
{
    SectorCount = ReadDiskSize(MountedDrivePort);
    Sectors = (bool*)kmalloc(SectorCount);
    memset(Sectors, false, SectorCount);
    if (Read(MountedDrivePort, 0, 0 >> 32, SectorCount / 512, Sectors))
    {
        GlobalRenderer->PrintNL("Initialized drive");
    }
}

uint32_t GetFreeSector()
{
    for(uint32_t i = (SectorCount / 512) + 2; i < SectorCount; i++)
    {
        if (!Sectors[i]) {
            Sectors[i] = false;
            return i;
        }
    }
}

uint8_t FormatDrive()
{
    memset(Sectors, false, SectorCount);
    if (Write(MountedDrivePort, 0, 0 >> 32, SectorCount / 512, Sectors))
    {
        GlobalRenderer->PrintNL("=> Cleared Existing Tree");
    }

    DirectoryTableEntry* Table = (DirectoryTableEntry*)kmalloc(sizeof(DirectoryTableEntry));
    int numBlocks = (512-2*sizeof(uint32_t))/sizeof(uint32_t);
    memset(Table->Blocks, 0, 128);
    Table->Blocks[0] = GetFreeSector();


    FileMetaData* systemFile;
    systemFile->Name = "system";
    systemFile->FirstNode = GetFreeSector();
    systemFile->LastNode = GetFreeSector();
    
    if (Write(MountedDrivePort, (SectorCount / 512) + 1, (uint64_t)((SectorCount / 512) + 1) >> 32, 1, (void*)Table))
    {
        GlobalRenderer->PrintNL("=> Directory Table Created");

        if (Write(MountedDrivePort, Table->Blocks[0], (uint64_t)Table->Blocks[0] >> 32, 1, (void*)systemFile))
        {
            GlobalRenderer->PrintNL("=> System File Created");
        }
    }
    
    kfree(Table);

    return 1;
}

FileMetaData** FS::List()
{
    void** list = (void**)kmalloc(512);
    void* buf = (void*)kmalloc(512);
    if (Read(MountedDrivePort, (SectorCount / 512) + 1, (uint64_t)((SectorCount / 512) + 1) >> 32, 1, buf))
    {
        DirectoryTableEntry* e = (DirectoryTableEntry*)buf;
        for (int i = 0; i < 128; i++)
        {
            if (e->Blocks[i] == 0) break;
            if (Read(MountedDrivePort, e->Blocks[i], (uint64_t)e->Blocks[i] >> 32, 1, buf))
            {
                FileMetaData* f = (FileMetaData*)buf;
                list[i] = f;
            }
        }

        kfree(buf);
    }

    return (FileMetaData**)list;
}

char* FS::Open(char* name)
{
    char* buf = (char*)kmalloc(512);
    if (Read(MountedDrivePort, (SectorCount / 512) + 1, (uint64_t)((SectorCount / 512) + 1) >> 32, 1, buf))
    {
        DirectoryTableEntry* e = (DirectoryTableEntry*)buf;
        FileMetaData* f;
        for (int i = 0; i < 128; i++)
        {
            f = (FileMetaData*)e->Blocks[i];
            if ((uint64_t)f->Name == (uint64_t)name) 
            {
                if (Read(MountedDrivePort, f->FirstNode, (uint64_t)(f->FirstNode) >> 32, 1, buf))
                {       
                   return buf; 
                }
            }
        }

        kfree(buf);
    }

    return NULL;
}

bool FS::WriteBuffer(char* name, char* wbuf)
{
    char* buf = (char*)kmalloc(512);
    if (Read(MountedDrivePort, (SectorCount / 512) + 1, (uint64_t)((SectorCount / 512) + 1) >> 32, 1, buf))
    {
        DirectoryTableEntry* e = (DirectoryTableEntry*)buf;
        FileMetaData* f;
        for (int i = 0; i < 128; i++)
        {
            f = (FileMetaData*)e->Blocks[i];
            if ((uint64_t)f->Name == (uint64_t)name) 
            {
                GlobalRenderer->PrintNL("here");
                if (Write(MountedDrivePort, f->FirstNode, (uint64_t)(f->FirstNode) >> 32, 1, (void*)wbuf))
                {       
                   return true; 
                }
            }
        }

        kfree(buf);
    }

    return false;
}