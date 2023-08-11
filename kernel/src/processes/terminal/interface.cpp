#include "interface.h"

char* commandPrompt;

void TakePrompt()
{
    EnableKeyboard(true);
    ReceiveInput(commandPrompt);
}

void InitializeTerminal()
{
    commandPrompt = (char*)kmalloc(0x100);
    GlobalRenderer->NextLine();
    GlobalRenderer->color = 0xCECE5A;
    GlobalRenderer->Print("Shell:/ > ");
    GlobalRenderer->color = 0xffffffff;
    TakePrompt();
}

void ExecutePrompt()
{
    EnableKeyboard(false);
    
    if (strcmp(commandPrompt, (const  char*)"memory", 6))
    {
        if (strcmp(commandPrompt + 7, (const  char*)"info", 4)) 
        {
            GlobalRenderer->color = 0xFEC260;
            GlobalRenderer->PrintNL("--- Memory Information ---");
            GlobalRenderer->color = 0x73BBC9;
            GlobalRenderer->Print("Used RAM: ");
            GlobalRenderer->color = 0xffffffff;
            PrintMemorySize(GlobalAllocator.GetUsedRAM());
            GlobalRenderer->NextLine();
            GlobalRenderer->color = 0x73BBC9;
            GlobalRenderer->Print("Reserved RAM: ");
            GlobalRenderer->color = 0xffffffff;
            PrintMemorySize(GlobalAllocator.GetReservedRAM());
            GlobalRenderer->NextLine();
            GlobalRenderer->color = 0x73BBC9;
            GlobalRenderer->Print("Free RAM: ");
            GlobalRenderer->color = 0xffffffff;
            PrintMemorySize(GlobalAllocator.GetFreeRAM());
            GlobalRenderer->NextLine();
        }
    }
    else 
    if (strcmp(commandPrompt, (const  char*)"drives", 6))
    {
        if (strcmp(commandPrompt + 7, (const  char*)"list", 4)) 
        {
            GlobalRenderer->color = 0xFEC260;
            GlobalRenderer->PrintNL("--- Connected Hard Drives ---");
            PrintDrives();
        }
        else
        if (strcmp(commandPrompt + 7, (const  char*)"mount", 5))
        {
            MountedDrivePort = atoi(commandPrompt + 13);
            GlobalRenderer->Print(to_string((int64_t)MountedDrivePort));
            GlobalRenderer->Print(" - Drive Mounted");
            GlobalRenderer->NextLine();
        }
        else
        if (strcmp(commandPrompt + 7, (const  char*)"format", 6))
        {
            uint8_t status = FormatDrive();
            if(status > 0)
                GlobalRenderer->PrintNL("Successfully Formatted Drive");
            else if(status < 0)
                GlobalRenderer->PrintNL("Drive already Formatted");
        }
        else
        if (strcmp(commandPrompt + 7, (const  char*)"size", 4))
        {
            int d = MountedDrivePort;
            if (d >= 0)
            {
                PrintMemorySize(ReadDiskSize(d)*512);
                GlobalRenderer->NextLine();
            }
            else
            {
                GlobalRenderer->PrintNL("No Drive is Mounted");
            }
        }
    }
    else 
    if (strcmp(commandPrompt, (const  char*)"fs", 2))
    {
        FS fs;
        if (strcmp(commandPrompt + 3, (const  char*)"list", 4))
        {
            int d = MountedDrivePort;
            if (d >= 0)
            {
                
                FileMetaData** l = fs.List();
                int i = 0;
                while (l[i])
                {
                    GlobalRenderer->PrintNL(l[i]->Name);
                    i++;
                }
            }
            else
            {
                GlobalRenderer->PrintNL("No Drive is Mounted");
            }
        }
        else 
        if (strcmp(commandPrompt + 3, (const  char*)"open", 4))
        {
            int d = MountedDrivePort;
            if (d >= 0)
            {
                char* buf = fs.Open(commandPrompt + 8);
                GlobalRenderer->PrintNL(buf);
            }
            else
            {
                GlobalRenderer->PrintNL("No Drive is Mounted");
            }
        }
        else 
        if (strcmp(commandPrompt + 3, (const  char*)"write", 5))
        {
            int d = MountedDrivePort;
            if (d >= 0)
            {
                if(fs.WriteBuffer(commandPrompt + 9, "hello world"))
                {
                    GlobalRenderer->PrintNL("Written successfully");
                }
            }
            else
            {
                GlobalRenderer->PrintNL("No Drive is Mounted");
            }
        }
    }

    GlobalRenderer->color = 0xCECE5A;
    GlobalRenderer->Print("Shell:/ > ");
    GlobalRenderer->color = 0xffffffff;
    TakePrompt();
}