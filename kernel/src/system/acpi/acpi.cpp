#include "acpi.h"
#include "../../BasicRenderer.h"
#include "../../cstr.h"



namespace ACPI {
    
    bool doChecksum(ACPISDTHeader *tableHeader)
    {
        unsigned char sum = 0;
    
        for (uint32_t i = 0; i < tableHeader->Length; i++)
        {
            sum += ((char *) tableHeader)[i];
        }
    
        return sum == 0;
    }

    void* FindTable(RSDPDescriptor* rsdp, char* signature)
    {
        if (rsdp->Revision == 2 && rsdp->XsdtAddress)
        {
            ACPI::ACPISDTHeader* xsdt = (ACPI::ACPISDTHeader*)rsdp->XsdtAddress;
            if (xsdt->Length < sizeof(ACPI::ACPISDTHeader)) {
                return 0;
            }

            if(!doChecksum(xsdt)) 
            {
                return 0;
            }

            int total = (xsdt->Length - sizeof(ACPISDTHeader)) / 8;
            for (int t = 0; t < total; t++)
            {
                ACPI::ACPISDTHeader* newHeader = (ACPI::ACPISDTHeader*)*(uint64_t*)((uint64_t)xsdt + sizeof(ACPI::ACPISDTHeader) + (t * 8));
                if (newHeader->Signature)
                {
                    for (uint8_t i = 0; i < 4; i++)
                    {
                        if (newHeader->Signature[i] != signature[i])
                        {
                            break;
                        }
                        if (i == 3) return newHeader;
                    }
                }
            }
        }

        return 0;
    }
}




