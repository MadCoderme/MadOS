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
                GlobalRenderer->PrintNL("SATA Drive Port Rebased");
                char* buff = (char*)kmalloc(1024);
                if (read(&abar->ports[i], (uint32_t)0, (uint32_t)(0 >> 32), 4, buff))
                {
                    GlobalRenderer->PrintNL("Successfully read");
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

bool read(HBAPort *port, uint32_t startl, uint32_t starth, uint32_t count, void *buf)
{
	port->is = (uint32_t) -1;		// Clear pending interrupt bits
	int spin = 0;                   // Spin lock timeout counter
	int slot = find_cmdslot(port);
	if (slot == -1)
		return false;
 
	HBA_CMD_HEADER *cmdheader = (HBA_CMD_HEADER*)port->clb;
	cmdheader += slot;
	cmdheader->cfl = sizeof(FIS_REG_H2D)/sizeof(uint32_t);	// Command FIS size
	cmdheader->w = 0;		// Read from device
	cmdheader->prdtl = (uint16_t)((count-1)>>4) + 1;	// PRDT entries count
    
    

	HBA_CMD_TBL *cmdtbl = (HBA_CMD_TBL*)(cmdheader->ctba);
	memset(cmdtbl, 0, sizeof(HBA_CMD_TBL) +
 		(cmdheader->prdtl-1)*sizeof(HBA_PRDT_ENTRY));
    
    int i = 0;
	// 8K bytes (16 sectors) per PRDT
	for (; i<cmdheader->prdtl-1; i++)
	{
		cmdtbl->prdt_entry[i].dba = (uint32_t)(uint64_t) buf;
		cmdtbl->prdt_entry[i].dbc = 8*1024-1;	// 8K bytes (this value should always be set to 1 less than the actual value)
		cmdtbl->prdt_entry[i].i = 1;
		buf += 4*1024;	// 4K words
		count -= 16;	// 16 sectors
	}
	// Last entry
	cmdtbl->prdt_entry[i].dba = (uint32_t)(uint64_t) buf;
	cmdtbl->prdt_entry[i].dbc = (count<<9)-1;	// 512 bytes per sector
	cmdtbl->prdt_entry[i].i = 1;
    



	// Setup command
	FIS_REG_H2D *cmdfis = (FIS_REG_H2D*)(&cmdtbl->cfis);
 
	cmdfis->fis_type = FIS_TYPE_REG_H2D;
	cmdfis->c = 1;	// Command
	cmdfis->command = ATA_CMD_READ_DMA_EX;
 
	cmdfis->lba0 = (uint8_t)startl;
	cmdfis->lba1 = (uint8_t)(startl>>8);
	cmdfis->lba2 = (uint8_t)(startl>>16);
	cmdfis->device = 1<<6;	// LBA mode
 
	cmdfis->lba3 = (uint8_t)(startl>>24);
	cmdfis->lba4 = (uint8_t)starth;
	cmdfis->lba5 = (uint8_t)(starth>>8);
 
	cmdfis->countl = count & 0xFF;
	cmdfis->counth = (count >> 8) & 0xFF;

 
	// The below loop waits until the port is no longer busy before issuing a new command
	while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
	{
		spin++;
	}
	if (spin == 1000000)
	{
		GlobalRenderer->PrintNL("Port is hung\n");
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
		GlobalRenderer->PrintNL("Read disk error\n");
		return false;
	}
 
	return true;
}
 
