#include "ahci.h"
#include "../../BasicRenderer.h"
#include "../../cstr.h"
#include "../../mem/heap.h"


HBAMem* GlobalABAR;

static int CheckType(HBAPort *port)
{
	uint32_t ssts = port->ssts;
 
	uint8_t ipm = (ssts >> 8) & 0x0F; // Interface power management
	uint8_t det = ssts & 0x0F; // Device detection
 
	if (det != HBA_PORT_DET_PRESENT)	// Check drive status
		return AHCI_DEV_NULL;
	if (ipm != HBA_PORT_IPM_ACTIVE)
		return AHCI_DEV_NULL;
 
	switch (port->sig)
	{
	case SATA_SIG_ATAPI:
		return AHCI_DEV_SATAPI;
	case SATA_SIG_SEMB:
		return AHCI_DEV_SEMB;
	case SATA_SIG_PM:
		return AHCI_DEV_PM;
	default:
		return AHCI_DEV_SATA;
	}
}

void PrintDrives()
{
    uint32_t pi = GlobalABAR->pi;
    int i = 0;
    while (i < 32)
    {
        if (pi & 1)
        {
            int dt = CheckType(&GlobalABAR->ports[i]);
            if (dt == AHCI_DEV_SATA)
            {
                GlobalRenderer->color = 0x73BBC9;
                GlobalRenderer->Print(to_string((int64_t)i));
                GlobalRenderer->color = 0xffffffff;
                GlobalRenderer->Print("  SATA Drive");
                GlobalRenderer->NextLine();

                if(i == 1)
                {
                //     char* wbuff = (char*)kmalloc(0x100);
                //     memset(wbuff, 65, 0x100);
                //     if (Write(&GlobalABAR->ports[i], 1, 1 >> 32, 5, wbuff))
                //     {
                //         GlobalRenderer->PrintNL("Successfully wrote");
                //     }

                    // char* buff = (char*)kmalloc(0x100);
                    // memset(buff, 0, 0x100);
                    // if (Read(&GlobalABAR->ports[i], 1, 1 >> 32, 5, buff))
                    // {
                    //     GlobalRenderer->PrintNL("Successfully read");
                    //     for (int t = 0; t < 512; t++){
                    //         GlobalRenderer->PutChar(buff[t]);
                    //     }
                    //     GlobalRenderer->NextLine();
                    // }
                }
            }
            else if (dt == AHCI_DEV_SATAPI)
            {
                GlobalRenderer->color = 0x73BBC9;
                GlobalRenderer->Print(to_string((int64_t)i));
                GlobalRenderer->color = 0xffffffff;
                GlobalRenderer->Print("  SATAPI Drive");
                GlobalRenderer->NextLine();
            }
        }
        
        pi >>= 1;
        i++;
    }
}

// Start command engine
void startCmd(HBAPort *port)
{
	// Wait until CR (bit15) is cleared
	while (port->cmd & HBA_PxCMD_CR)
		;
 
	// Set FRE (bit4) and ST (bit0)
	port->cmd |= HBA_PxCMD_FRE;
	port->cmd |= HBA_PxCMD_ST; 
}
 
// Stop command engine
void stopCmd(HBAPort *port)
{
	// Clear ST (bit0)
	port->cmd &= ~HBA_PxCMD_ST;
 
	// Clear FRE (bit4)
	port->cmd &= ~HBA_PxCMD_FRE;
 
	// Wait until FR (bit14), CR (bit15) are cleared
	while(1)
	{
		if (port->cmd & HBA_PxCMD_FR)
			continue;
		if (port->cmd & HBA_PxCMD_CR)
			continue;
		break;
	}
 
}

void PortRebase(HBAPort *port, int portno)
{
	stopCmd(port);	// Stop command engine
 
        void* newBase = GlobalAllocator.RequestPage();
        port->clb = (uint32_t)(uint64_t)newBase;
        port->clbu = (uint32_t)((uint64_t)newBase >> 32);
        memset((void*)(uint64_t)(port->clb), 0, 1024);

        void* fisBase = GlobalAllocator.RequestPage();
        port->fb = (uint32_t)(uint64_t)fisBase;
        port->fbu = (uint32_t)((uint64_t)fisBase >> 32);
        memset(fisBase, 0, 256);

        HBA_CMD_HEADER* cmdHeader = (HBA_CMD_HEADER*)((uint64_t)port->clb + ((uint64_t)port->clbu << 32));

        for (int i = 0; i < 32; i++){
            cmdHeader[i].prdtl = 8;

            void* cmdTableAddress = GlobalAllocator.RequestPage();
            uint64_t address = (uint64_t)cmdTableAddress + (i << 8);
            cmdHeader[i].ctba = (uint32_t)(uint64_t)address;
            cmdHeader[i].ctbau = (uint32_t)((uint64_t)address >> 32);
            memset(cmdTableAddress, 0, 256);
        }
 
	startCmd(port);	// Start command engine
}


void ProbePort(HBAMem* abar)
{
    GlobalPTM.MapMemory(abar, abar);
    GlobalABAR = abar;
    uint32_t pi = abar->pi;
    int i = 0;
    while (i < 32)
    {
        if (pi & 1)
        {
            int dt = CheckType(&abar->ports[i]);
            if (dt == AHCI_DEV_SATA)
            {
                // GlobalRenderer->PrintNL("SATA Drive Found");
                PortRebase(&abar->ports[i], i);

                // char* wbuff = (char*)GlobalAllocator.RequestPage();
                // memset(wbuff, 65, 0x1000);
                // if (write(&abar->ports[0], 2, 2 >> 32, 20, wbuff))
                // {
                //     GlobalRenderer->PrintNL("Successfully wrote");
                // }

                // char* buff = (char*)GlobalAllocator.RequestPage();
                // memset(buff, 0, 0x1000);
                // if (read(&abar->ports[0], 2, 2 >> 32, 20, buff))
                // {
                //     GlobalRenderer->PrintNL("Successfully read");
                //     for (int t = 0; t < 1024; t++){
                //         GlobalRenderer->PutChar(buff[t]);
                //     }
                //     GlobalRenderer->NextLine();
                // }
            }
            else if (dt == AHCI_DEV_SATAPI)
            {
                // GlobalRenderer->PrintNL("SATAPI Drive Found");
            }
            else if (dt == AHCI_DEV_SEMB)
            {
                // trace_ahci("SEMB drive found at port %d\n", i);
            }
            else if (dt == AHCI_DEV_PM)
            {
                // trace_ahci("PM drive found at port %d\n", i);
            }
            else
            {
                // GlobalRenderer->Print("No Drive");
            }
        }
        
        pi >>= 1;
        i++;
    }
}

// Find a free command list slot
int find_cmdslot(HBAPort *port)
{
	// If not set in SACT and CI, the slot is free
	uint32_t slots = (port->sact | port->ci);
    int cmdslots = (GlobalABAR->cap & 0x0f00) >> 8;
	for (int i=0; i<cmdslots; i++)
	{
		if ((slots&1) == 0)
			return i;
		slots >>= 1;
	}
	GlobalRenderer->PrintNL("Cannot find free command list entry\n");
	return -1;
}

bool Read(int i, uint32_t startl, uint32_t starth, uint32_t count, void* buf)
{
    HBAPort* port = (HBAPort*)(&GlobalABAR->ports[i]);

    port->is = (uint32_t)-1; // Clear pending interrupt bits
    int slot = find_cmdslot(port);
	if (slot == -1)
		return false;

        HBA_CMD_HEADER* cmdHeader = (HBA_CMD_HEADER*)port->clb;
        cmdHeader += slot;
        cmdHeader->cfl = sizeof(FIS_REG_H2D)/ sizeof(uint32_t); //command FIS size;
        cmdHeader->w = 0; //this is a read
        cmdHeader->prdtl = 1;

        HBA_CMD_TBL* commandTable = (HBA_CMD_TBL*)(cmdHeader->ctba);
        memset(commandTable, 0, sizeof(HBA_CMD_TBL) + (cmdHeader->prdtl-1)*sizeof(HBA_PRDT_ENTRY));

        commandTable->prdt_entry[0].dba = (uint32_t)(uint64_t)buf;
        commandTable->prdt_entry[0].dbau = (uint32_t)((uint64_t)buf >> 32);
        commandTable->prdt_entry[0].dbc = (count<<9)-1; // 512 bytes per sector
        commandTable->prdt_entry[0].i = 1;

        FIS_REG_H2D* cmdFIS = (FIS_REG_H2D*)(&commandTable->cfis);

        cmdFIS->fis_type = FIS_TYPE_REG_H2D;
        cmdFIS->c = 1; // command
        cmdFIS->command = ATA_CMD_READ_DMA_EX;

        cmdFIS->lba0 = (uint8_t)startl;
        cmdFIS->lba1 = (uint8_t)(startl >> 8);
        cmdFIS->lba2 = (uint8_t)(startl >> 16);
        cmdFIS->lba3 = (uint8_t)starth;
        cmdFIS->lba4 = (uint8_t)(starth >> 8);
        cmdFIS->lba5 = (uint8_t)(starth >> 16);

        cmdFIS->device = 1<<6; //LBA mode

        cmdFIS->countl = count & 0xFF;
        cmdFIS->counth = (count >> 8) & 0xFF;

        uint64_t spin = 0;

        while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000){
            spin ++;
        }
        if (spin == 1000000) {
            return false;
        }
 
	port->ci = 1<<slot;	// Issue command
 
	// Wait for completion
	while (1)
	{
		// In some longer duration reads, it may be helpful to spin on the DPS bit 
		// in the PxIS port field as well (1 << 5)
		if ((port->ci & (1<<slot)) == 0) 
			break;
		if (port->is & HBA_PxIS_TFES)	// Task file error
		{
			GlobalRenderer->PrintNL("Read disk error\n");
			return false;
		}
	}
    

	// Check again
	if (port->is & HBA_PxIS_TFES)
	{
		GlobalRenderer->PrintNL("Read disk error here");
		return false;
	}
 
	return true;
}
 
bool Write(int i, uint32_t startl, uint32_t starth, uint32_t count, void *buf)
{
    HBAPort* port = (HBAPort*)(&GlobalABAR->ports[i]);
    port->is = (uint32_t)-1;       // Clear pending interrupt bits
    int spin = 0; // Spin lock timeout counter
    int slot = find_cmdslot(port);

    //HBA_CMD_HEADER *cmdheader = (HBA_CMD_HEADER*) (KERN_VMBASE + port->clb);
    HBA_CMD_HEADER* cmdheader = (HBA_CMD_HEADER*)port->clb;
    cmdheader += slot;

    cmdheader->cfl = sizeof(FIS_REG_H2D)/sizeof(uint32_t); // Command FIS size
    cmdheader->w = 1;       // Write device
    cmdheader->prdtl = (uint16_t)((count-1)>>4) + 1;    // PRDT entries count

    HBA_CMD_TBL *cmdtbl = (HBA_CMD_TBL*)cmdheader->ctba;
    memset(cmdtbl, 0, sizeof(HBA_CMD_TBL) + (cmdheader->prdtl-1)*sizeof(HBA_PRDT_ENTRY));


    cmdtbl->prdt_entry[0].dba = (uint32_t)(uint64_t)buf;
    cmdtbl->prdt_entry[0].dbau = (uint32_t)((uint64_t)buf >> 32);
    cmdtbl->prdt_entry[0].dbc = (count<<9)-1;   // 512 bytes per sector

    // Setup command
    FIS_REG_H2D *cmdfis = (FIS_REG_H2D*)(&cmdtbl->cfis);

    cmdfis->fis_type = FIS_TYPE_REG_H2D;
    cmdfis->c = 1;  // Command
    cmdfis->command = ATA_CMD_WRITE_DMA_EX;

    cmdfis->lba0 = (uint8_t)startl;
    cmdfis->lba1 = (uint8_t)(startl>>8);
    cmdfis->lba2 = (uint8_t)(startl>>16);
    cmdfis->device = 1<<6;  // LBA mode

    cmdfis->lba3 = (uint8_t)(startl>>24);
    cmdfis->lba4 = (uint8_t)starth;
    cmdfis->lba5 = (uint8_t)(starth>>8);

    cmdfis->countl = (count & 0xff);
    cmdfis->counth = (count >> 8);

    // The below loop waits until the port is no longer busy before issuing a new command
    while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
    {
        spin++;
    }
    if (spin == 1000000)
    {
        return false;
    }

    port->ci = (1<<slot); // Issue command

    // Wait for completion
    while (1)
    {

        if ((port->ci & (1<<slot)) == 0)
            break;
        if (port->is & HBA_PxIS_TFES)   // Task file error
        {
            GlobalRenderer->PrintNL("Write disk error here");
            return false;
        }
    }

    // Check again
    if (port->is & HBA_PxIS_TFES)
    {
        GlobalRenderer->PrintNL("Write disk error here");
        return false;
    }

    return true;
}

uint64_t ReadDiskSize(int i)
{
    char* buf = (char*)kmalloc(0x4096);
    SATA_ident* data_base;
    HBAPort* port = (HBAPort*)(&GlobalABAR->ports[i]);

    HBA_CMD_HEADER *cmdhead=(HBA_CMD_HEADER*)port->clb;
    cmdhead->cfl = 5;
    cmdhead->w = 0;
    cmdhead->prdtl = 1;
    cmdhead->p = 1;
    cmdhead->c = 1;

    /***Make the Command Table***/
    HBA_CMD_TBL *cmdtbl = (HBA_CMD_TBL*)cmdhead->ctba;
    memset((void*)cmdtbl, 0, sizeof(HBA_CMD_TBL));
    cmdtbl->prdt_entry[0].dba = (uint32_t)(uint64_t)buf;
    cmdtbl->prdt_entry[0].dbc = 0x1FF;
    cmdtbl->prdt_entry[0].i = 1;   // interrupt when identify complete
    data_base = (SATA_ident*)cmdtbl->prdt_entry[0].dba;
    memset((void*)data_base, 0, 4096);


    FIS_REG_H2D *cmdfis = (FIS_REG_H2D*)cmdtbl->cfis;
    memset((void*)cmdfis,0,sizeof(FIS_REG_H2D));
    cmdfis->fis_type = FIS_TYPE_REG_H2D;
    cmdfis->c = 1;
    cmdfis->command = ATA_CMD_IDENTIFY;
 
	port->ci = 1;	// Issue command
 
	// Wait for completion
	while (1)
	{
		// In some longer duration reads, it may be helpful to spin on the DPS bit 
		// in the PxIS port field as well (1 << 5)
		if (port->ci == 0) 
			break;
		if (port->is & HBA_PxIS_TFES)	// Task file error
		{
			GlobalRenderer->PrintNL("error");
		}
	}
    

	// Check again
	if (port->is & HBA_PxIS_TFES)
	{
		GlobalRenderer->PrintNL("error");
	}
    
    kfree(buf);
	return data_base->lba_capacity;
}