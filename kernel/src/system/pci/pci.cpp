#include "pci.h"
#include "../../driver/ahci/ahci.h"

uint8_t PciRead8(unsigned int id, unsigned int reg)
{
    uint32_t addr = 0x80000000 | id | (reg & 0xfc);
    outl(PCI_CONFIG_ADDR, addr);
    return inb(PCI_CONFIG_DATA + (reg & 0x03));
}

uint16_t PciRead16(unsigned int id, unsigned int reg)
{
    uint32_t addr = 0x80000000 | id | (reg & 0xfc);
    outl(PCI_CONFIG_ADDR, addr);
    return inw(PCI_CONFIG_DATA + (reg & 0x02));
}

uint32_t PciRead32(unsigned int id, unsigned int reg)
{
    uint32_t addr = 0x80000000 | id | (reg & 0xfc);
    outl(PCI_CONFIG_ADDR, addr);
    return inl(PCI_CONFIG_DATA);
}

void PciWrite8(unsigned int id, unsigned int reg, uint8_t data)
{
    uint32_t address = 0x80000000 | id | (reg & 0xfc);
    outl(PCI_CONFIG_ADDR, address);
    outb(PCI_CONFIG_DATA + (reg & 0x03), data);
}

void PciWrite16(unsigned int id, unsigned int reg, uint16_t data)
{
    uint32_t address = 0x80000000 | id | (reg & 0xfc);
    outl(PCI_CONFIG_ADDR, address);
    outw(PCI_CONFIG_DATA + (reg & 0x02), data);
}

void PciWrite32(unsigned int id, unsigned int reg, uint32_t data)
{
    uint32_t address = 0x80000000 | id | (reg & 0xfc);
    outl(PCI_CONFIG_ADDR, address);
    outl(PCI_CONFIG_DATA, data);
}

namespace PCIExpress 
{   
    void PciVisit(uint16_t bus, uint16_t dev, uint8_t func)
    {
        uint32_t id = PCI_MAKE_ID(bus, dev, func);
        
        PCI_T info;
        info.vendorId = PciRead16(id, PCI_CONFIG_VENDOR_ID);
        if (info.vendorId == 0xffff)
        {
            return;
        }

        info.deviceId = PciRead16(id, PCI_CONFIG_DEVICE_ID);
        info.progIntf = PciRead8(id, PCI_CONFIG_PROG_INTF);
        info.subclass = PciRead8(id, PCI_CONFIG_SUBCLASS);
        info.classCode = PciRead8(id, PCI_CONFIG_CLASS_CODE);

        info.BAR5 = PciRead32(id, PCI_CONFIG_BAR5);


        // GlobalRenderer->Print(getVendorName(info.vendorId));
        // GlobalRenderer->Print(" | ");
        // GlobalRenderer->Print(getFunction(info.classCode, info.subclass));
        // GlobalRenderer->Print(" | ");
        // GlobalRenderer->Print(getDeviceName(info.vendorId, info.deviceId));
        // GlobalRenderer->Print(" | ");
        // // GlobalRenderer->Print(to_hstring(info.headerType));
        // GlobalRenderer->NextLine();

        if (info.classCode == 0x01 && info.subclass == 0x6 && info.progIntf == 0x1) // AHCI Interface
        {
            HBAMem* abar = (HBAMem*)info.BAR5;
            ProbePort(abar);            
        }

        // const PciDriver *driver = g_pciDriverTable;
        // while (driver->init)
        // {
        //     driver->init(id, &info);
        //     ++driver;
        // }
    }

    void EnumeratePIC(ACPI::ACPIMCFGHeader* mcfg)
    {
        GlobalRenderer->NextLine();
        for (uint16_t bus = 0; bus < 256; ++bus)
        {
            for (uint16_t dev = 0; dev < 32; ++dev)
            {
                uint32_t baseId = PCI_MAKE_ID(bus, dev, 0);
                uint8_t headerType = PciRead8(baseId, PCI_CONFIG_HEADER_TYPE);
                uint8_t funcCount = headerType & PCI_TYPE_MULTIFUNC ? 8 : 1;
                for (uint8_t func = 0; func < funcCount; ++func)
                {
                    PciVisit(bus, dev, func);
                }
            }
        }

    }
}