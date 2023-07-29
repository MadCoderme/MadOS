#include "ahci.h"
#include "../BasicRenderer.h"
#include "../cstr.h"
#include "../mem/heap.h"


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
 

	// Command list offset: 1K*portno
	// Command list entry size = 32
	// Command list entry maxim count = 32
	// Command list maxim size = 32*32 = 1K per port
	port->clb = AHCI_BASE + (portno<<10);
	port->clbu = 0;
	memset((void*)(port->clb), 0, 1024);
 
	// FIS offset: 32K+256*portno
	// FIS entry size = 256 bytes per port
	port->fb = AHCI_BASE + (32<<10) + (portno<<8);
	port->fbu = 0;
	memset((void*)(port->fb), 0, 256);
 
	// Command table offset: 40K + 8K*portno
	// Command table size = 256*32 = 8K per port
	HBA_CMD_HEADER *cmdheader = (HBA_CMD_HEADER*)(port->clb);
	for (int i=0; i<32; i++)
	{
		cmdheader[i].prdtl = 8;	// 8 prdt entries per command table
					// 256 bytes per command table, 64+16+48+16*8
		// Command table offset: 40K + 8K*portno + cmdheader_index*256
		cmdheader[i].ctba = AHCI_BASE + (40<<10) + (portno<<13) + (i<<8);
		cmdheader[i].ctbau = 0;
		memset((void*)cmdheader[i].ctba, 0, 256);
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
                GlobalRenderer->PrintNL("SATA Drive Found");
                PortRebase(&abar->ports[i], i);
                char* buff = (char*)GlobalAllocator.RequestPage();
                memset(buff, 0, 0x1000);

                if (read(&abar->ports[i], (uint32_t)0, (uint32_t)(0 >> 32), 1, buff))
                {
                    GlobalRenderer->PrintNL("Successfully read");
                    for (int t = 0; t < 1024; t++){
                        GlobalRenderer->PutChar(buff[t]);
                    }
                }
            }
            else if (dt == AHCI_DEV_SATAPI)
            {
                GlobalRenderer->PrintNL("SATAPI Drive Found");
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

bool read(HBAPort *port, uint32_t startl, uint32_t starth, uint32_t count, void* buf)
{
    uint64_t sector = 0;
	uint32_t sectorL = (uint32_t) sector;
        uint32_t sectorH = (uint32_t) (sector >> 32);

        int slot = find_cmdslot(port);
        if (slot == -1)
            return false;

        port->is = (uint32_t)-1; // Clear pending interrupt bits

        HBA_CMD_HEADER* cmdHeader = (HBA_CMD_HEADER*)port->clb;
        cmdHeader += slot;
        cmdHeader->cfl = sizeof(FIS_REG_H2D)/ sizeof(uint32_t); //command FIS size;
        cmdHeader->w = 0; //this is a read
        cmdHeader->prdtl = 1;

        HBA_CMD_TBL* commandTable = (HBA_CMD_TBL*)(cmdHeader->ctba);
        memset(commandTable, 0, sizeof(HBA_CMD_TBL) + (cmdHeader->prdtl-1)*sizeof(HBA_PRDT_ENTRY));

        int i;
        // 8K bytes (16 sectors) per PRDT
        for (i=0; i<cmdHeader->prdtl-1; i++)
        {
            commandTable->prdt_entry[i].dba = (uint32_t)(uint64_t)buf;
            commandTable->prdt_entry[i].dbau = (uint32_t)(uint64_t)(buf) >> 32;
            commandTable->prdt_entry[i].dbc = 8*1024; // 8K bytes
            commandTable->prdt_entry[i].i = 1;
            buf += 4*1024;  // 4K words
            count -= 16;    // 16 sectors
        }
        // Last entry
        commandTable->prdt_entry[i].dba = (uint32_t)(uint64_t)buf;
        commandTable->prdt_entry[i].dbau = (uint32_t)(uint64_t)(buf) >> 32;
        //printk("dba & dbau: %p %p\n", commandTable ->prdt_entry[i].dba, commandTable -> prdt_entry[i].dbau);
        commandTable->prdt_entry[i].dbc = count<<9;   // 512 bytes per sector
        commandTable->prdt_entry[i].i = 1;

        FIS_REG_H2D* cmdFIS = (FIS_REG_H2D*)(&commandTable->cfis);

        cmdFIS->fis_type = FIS_TYPE_REG_H2D;
        cmdFIS->control = 1; // command
        cmdFIS->command = ATA_CMD_READ_DMA_EX;

        cmdFIS->lba0 = (uint8_t)sectorL;
        cmdFIS->lba1 = (uint8_t)(sectorL>>8);
        cmdFIS->lba2 = (uint8_t)(sectorL>>16);
        cmdFIS->device = 1<<6;	// LBA mode
    
        cmdFIS->lba3 = (uint8_t)(sectorH>>24);
        cmdFIS->lba4 = (uint8_t)sectorH;
        cmdFIS->lba5 = (uint8_t)(sectorH>>8);

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
	while (true)
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

        if (port->is & HBA_PxIS_TFES)
        {
            GlobalRenderer->PrintNL("Read disk error\n");
            return false;
        }

        return true;
}
 
