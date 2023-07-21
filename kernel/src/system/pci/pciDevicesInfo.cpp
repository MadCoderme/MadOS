#include "pci.h"

    const char* getVendorName(uint16_t vendorId) 
    {
        switch (vendorId)
        {
        case 0x8086:
            return "Intel";
        case 0x1028:
            return "Dell";
        case 0x1043:
            return "Asus";
        case 0x1022:
            return "AMD";
        case 0x1025:
            return  "Acer";
        default:
            return "Unknown";
        }
    }

    const char* getFunction(uint16_t classCode, uint16_t subClass) 
    {
        switch (classCode)
        {
        case 0x1:
            switch (subClass)
            {
            case 0x6:
                return "Serial ATA Controller";
            case 0x8:
                return "Storage Controller";
            default:
                return to_hstring(subClass);
            }
        case 0x2:
            switch (subClass)
            {
            case 0x0:
                return "Ethernet Controller";
            case 0x1:
                return "Token Ring Controller";
            case 0x2:
                return "FDDI Controller";
            case 0x3:
                return "ATM Controller";
            case 0x4:
                return "ISDN Controller"; 
            default:
                return to_hstring(subClass);
            }
        case 0x3:
            if (subClass == 0x0) return "VGA Controller";
            return "Display Controller";
        case 0x6:
            switch (subClass)
            {
            case 0x0:
                return "Host Bridge";
            case 0x1:
                return "ISA Bridge";
            case 0x4:
                return "PCI-to-PCI Bridge";
            case 0x9:
                return "PCI-to-PCI Bridge"; 
            default:
                return to_hstring(subClass);
            }

        case 0xc:
            if (subClass == 0x3) return "USB Controller";
            return "Serial Bus Cotroller";
        default:
            return to_hstring(classCode);
        }
    }

    const char* getDeviceName(uint16_t vendorID, uint16_t deviceID){
        switch (vendorID){
            case 0x8086: // Intel
                switch(deviceID){
                    case 0x29C0:
                        return "Express DRAM Controller";
                    case 0x2918:
                        return "LPC Interface Controller";
                    case 0x2922:
                        return "6 port SATA Controller [AHCI mode]";
                    case 0x2930:
                        return "SMBus Controller";
                }
        }
        return to_hstring(deviceID);
    }

    const char* getInterface(uint16_t classCode, uint16_t subClass, uint16_t progIf) 
    {
        switch (classCode)
        {
        case 0x1:
            switch (subClass)
            {
            case 0x6:
                switch (progIf)
                {
                case 0x0:
                    return "Vendor Specific Interface";
                case 0x1:
                    return "AHCI 1.0";
                case 0x2:
                    return "Serial Storage Bus";
                }
            default:
                return to_hstring(progIf);
            }
        case 0x6:
            if(subClass == 0x4) 
            {
                if(progIf == 0x0) return "Normal Decode";
                return "Subtractive Decode";
            }

        case 0xc:
            if (subClass == 0x3) 
            {
                switch (progIf)
                {
                case 0x0:
                    return "UHCI Controller";
                case 0x10:
                    return "OHCI Controller";
                case 0x20:
                    return "EHCI (USB2) Controller";
                case 0x30:
                    return "XHCI (USB3) Controller";
                case 0x80:
                    return "Unspecified";
                case 0xFE:
                    return "USB Device (Not a host controller)"; 
                }
            }
        default:
            return to_hstring(classCode);
        }
    }