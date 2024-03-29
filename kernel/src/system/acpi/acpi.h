#pragma once
#include <stdint.h>

namespace ACPI {

    struct RSDPDescriptor {
        char Signature[8];
        uint8_t Checksum;
        char OEMID[6];
        uint8_t Revision;
        uint32_t RsdtAddress;
        uint32_t Length;
        uint64_t XsdtAddress;
        uint8_t ExtendedChecksum;
        uint8_t reserved[3];
    } __attribute__ ((packed));

    struct ACPISDTHeader {
        char Signature[4];
        uint32_t Length;
        uint8_t Revision;
        uint8_t Checksum;
        char OEMID[6];
        char OEMTableID[8];
        uint32_t OEMRevision;
        uint32_t CreatorID;
        uint32_t CreatorRevision;
    }__attribute__ ((packed));

    struct acpi_pci_config_t
    {
        uint64_t base;
        uint16_t pciSegment;
        uint8_t startBus;
        uint8_t endBus;
        uint32_t reserved;
    }__attribute__ ((packed));

    struct ACPIMCFGHeader {
        ACPISDTHeader Header;
        uint64_t Reserved;
        acpi_pci_config_t buses[];
    }__attribute__ ((packed));

    void* FindTable(RSDPDescriptor* rsdp, char* signature);
};
