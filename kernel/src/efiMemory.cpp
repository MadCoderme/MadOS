#include "efiMemory.h"

const char* EFI_MEMORY_TYPE_STRINGS[] = {
    "EfiReservedMemoryType",
    "EfiLoaderCode",
    "EfiLoaderData",
    "EfiBootServicesCode",
    "EfiBootServicesData",
    "EfiRuntimeServiceCode", //
    "EfiRuntimeServicesData", //
    "EfiConventionalMemory",
    "EfiUnusableMemory", //
    "EfiACPIReclaimMemory",
    "EfiACPIMemoryNVS", //
    "EfiMemoryMappedIO", //
    "EfiMemoryMappedIOPortSpace", //
    "EfiPalCode"
};