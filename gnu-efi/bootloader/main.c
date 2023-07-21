#include <efi.h>
#include <efilib.h>
#include <elf.h>


typedef unsigned long long size_t;

typedef struct {
	void* BaseAddress;
	size_t BufferSize;
	unsigned int Width;
	unsigned int Height;
	unsigned int PixelsPerScanline;
} FrameBuffer;


#define PSF1_MAGIC0 0x36
#define PSF1_MAGIC1 0x04
 
typedef struct {
    unsigned char magic[2]; // Magic bytes for idnetiifcation.
    unsigned char fontMode; // PSF font mode
    unsigned char characterSize; // PSF character size.
} PSF1_Header;

typedef struct {
	PSF1_Header* fontHeader;
	void* glyphBuffer;
} PSF1_FONT;

typedef struct {
	FrameBuffer* frameBuffer;
	PSF1_FONT* font;
	EFI_MEMORY_DESCRIPTOR* memoryMap;
	UINTN mapSize, descriptorSize;
	void* rsdp;
} BootInfo;


FrameBuffer frameBuffer;
FrameBuffer* InitializeGOP() {
	EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
	EFI_STATUS status;

	status = uefi_call_wrapper(BS->LocateProtocol, 3, &gopGuid, NULL, (void**)&gop);
	if(EFI_ERROR(status)) 
	{
		Print(L"Unable to locate GOP\n\r");
		return NULL;
	}

	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* info;
	UINTN size, numModes, nativeMode;

	status = uefi_call_wrapper(gop->QueryMode, 4, gop, gop->Mode == NULL ? 0 : gop->Mode->Mode, &size, &info);
	if(status == EFI_NOT_STARTED)
	{
		status = uefi_call_wrapper(gop->SetMode, 2, gop, 0);
	}

	if(EFI_ERROR(status))
	{
		Print(L"Unable to get GOP Native mode\n\r");
		return NULL;
	}
	else 
	{
		nativeMode = gop->Mode->Mode;
		numModes = gop->Mode->MaxMode;

		Print(L"Native Mode: %d, Max Modes: %d\n\r", nativeMode, numModes);
	}

	frameBuffer.BaseAddress = (void*)gop->Mode->FrameBufferBase;
	frameBuffer.BufferSize = gop->Mode->FrameBufferSize;
	frameBuffer.Width = gop->Mode->Info->HorizontalResolution;
	frameBuffer.Height = gop->Mode->Info->VerticalResolution;
	frameBuffer.PixelsPerScanline = gop->Mode->Info->PixelsPerScanLine;

	return &frameBuffer;

}

EFI_FILE* LoadFile(EFI_FILE* Directory, CHAR16* Path, EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable ) {

	EFI_FILE* LoadedFile;
	
	EFI_LOADED_IMAGE_PROTOCOL* LoadedImage;
	SystemTable->BootServices->HandleProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, (void**)&LoadedImage);

	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* FileSystem;
	SystemTable->BootServices->HandleProtocol(LoadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void**)&FileSystem);

	if(Directory == NULL) {
		FileSystem->OpenVolume(FileSystem, &Directory);
	}

	EFI_STATUS status = Directory->Open(Directory, &LoadedFile, Path, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
	if(status != EFI_SUCCESS) {
		return NULL;
	}

	return LoadedFile;

}

PSF1_FONT* LoadFont(EFI_FILE* Directory, CHAR16* Path, EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
	EFI_FILE* font = LoadFile(Directory, Path, ImageHandle, SystemTable);
	if(font == NULL) {
		Print(L"Could not load Font\n\r");
		return NULL;
	}

	PSF1_Header* fontHeader;
	SystemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(PSF1_Header), (void**)&fontHeader);
	UINTN size = sizeof(PSF1_Header);
	font->Read(font, &size, fontHeader);

	if(fontHeader->magic[0] != PSF1_MAGIC0 && fontHeader->magic[1] != PSF1_MAGIC1) {
		Print(L"Magic Number did not match %d\n\r", fontHeader->magic[0]);
		return NULL;
	}	

	UINTN glyphBufferSize = fontHeader->characterSize * 256;
	if(fontHeader->fontMode == 1) {
		glyphBufferSize = fontHeader->characterSize * 512;
	}

	void* glyphBuffer;
	{
		font->SetPosition(font, sizeof(PSF1_Header));
		SystemTable->BootServices->AllocatePool(EfiLoaderData, glyphBufferSize, (void**)&glyphBuffer);
		font->Read(font, &glyphBufferSize, glyphBuffer);
	}

	PSF1_FONT* finalFont;
	SystemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(PSF1_FONT), (void**)&finalFont);
	finalFont->fontHeader = fontHeader;
	finalFont->glyphBuffer = glyphBuffer;

	return finalFont;
}


int memcmp(const void* aptr, const void* bptr, size_t n) {
	const unsigned char* a = aptr, *b = bptr;

	for(size_t i = 0; i < n; i++) {
		if(a[i] < b[i]) return -1;
		else if(a[i] > b[i]) return 1;
	}

	return 0;
}

UINTN strcmp(CHAR8* a, CHAR8* b, UINTN length){
	
	for (UINTN i = 0; i < length; i++)
	{
		if (*a != *b) return 0;
	}

	return 1;
}

EFI_STATUS efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
	
	InitializeLib(ImageHandle, SystemTable);
	
	EFI_FILE* Kernel = LoadFile(NULL, L"kernel.elf", ImageHandle, SystemTable);

	if(Kernel == NULL) {
		Print(L"Not loaded yet \n\r");
	} else {
		Print(L"Kernel File Successfully Loaded \n\r");
	}

	Elf64_Ehdr header;
	{
		UINTN FileInfoSize;
		EFI_FILE_INFO* FileInfo;
		Kernel->GetInfo(Kernel, &gEfiFileInfoGuid, &FileInfoSize, NULL);
		SystemTable->BootServices->AllocatePool(EfiLoaderData, FileInfoSize, (void**)&FileInfo);
		Kernel->GetInfo(Kernel, &gEfiFileInfoGuid, &FileInfoSize, (void**)&FileInfo);

		UINTN size = sizeof(header);
		Kernel->Read(Kernel, &size, &header);
	}

	if(
		memcmp(&header.e_ident[EI_MAG0], ELFMAG, SELFMAG) != 0 ||
		header.e_ident[EI_CLASS] != ELFCLASS64 ||
		header.e_ident[EI_DATA] != ELFDATA2LSB ||
		header.e_type != ET_EXEC ||
		header.e_machine != EM_X86_64 ||
		header.e_version != EV_CURRENT
	) {
		Print(L"Bad Kernel Format \n\r");
	} else {
		Print(L"Kernel header verified \n\r");
	}

	Elf64_Phdr* phdrs;
	{
		Kernel->SetPosition(Kernel, header.e_phoff);
		UINTN size = header.e_phnum * header.e_phentsize;
		SystemTable->BootServices->AllocatePool(EfiLoaderData, size, (void**)&phdrs);
		Kernel->Read(Kernel, &size, phdrs);
	}

	for(
		Elf64_Phdr* phdr = phdrs;
		(char*)phdr < (char*)phdrs + header.e_phnum * header.e_phentsize;
		phdr = (Elf64_Phdr*)((char*)phdr + header.e_phentsize)
	)
	{
		switch (phdr->p_type)
		{
		case PT_LOAD:
		{
			int pages = (phdr->p_memsz + 0x1000 - 1) / 0x1000;
			Elf64_Addr segment = phdr->p_paddr;
			SystemTable->BootServices->AllocatePages(AllocateAddress, EfiLoaderData, pages, &segment);

			Kernel->SetPosition(Kernel, phdr->p_offset);
			UINTN size = phdr->p_filesz;
			Kernel->Read(Kernel, &size, (void*)segment);

			break;
		}
		
		default:
			break;
		}
	}


	void (*KernelStart)(BootInfo*) = ((__attribute__((sysv_abi)) void (*)(BootInfo*) ) header.e_entry);
	
	Print(L"Kernel Info Loaded successfully\n\r");

	FrameBuffer* newBuffer = InitializeGOP();
	PSF1_FONT* font = LoadFont(NULL, L"font16.psf", ImageHandle, SystemTable);

	if(font == NULL) {
		Print(L"Failed to Load Font\n\r");
	}
	else 
	{
		Print(L"Font Loaded with size %d\n\r", font->fontHeader->characterSize);
	}

	EFI_STATUS status;
	EFI_MEMORY_DESCRIPTOR *EfiMemoryMap;
	UINTN MapSize, MapKey, DescriptorSize;
	UINT32 DescriptorVersion;

	MapSize = 0;
	EfiMemoryMap = NULL;
	{
		SystemTable->BootServices->GetMemoryMap(&MapSize, EfiMemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
		SystemTable->BootServices->AllocatePool(EfiLoaderData, MapSize, (void**)&EfiMemoryMap);
		status = SystemTable->BootServices->GetMemoryMap(&MapSize, EfiMemoryMap, &MapKey, &DescriptorSize, &DescriptorVersion);
		if(EFI_ERROR(status)) {
			Print(L"Failed to get memory map\n\r");
		}
	}

	EFI_CONFIGURATION_TABLE* configTable = SystemTable->ConfigurationTable;
	EFI_GUID acpiGuid = ACPI_20_TABLE_GUID;
	void* rsdp = NULL;

	for (UINTN i = 0; i < SystemTable->NumberOfTableEntries; i++)
	{
		if (CompareGuid(&configTable[i].VendorGuid, &acpiGuid))
		{
			if (strcmp((CHAR8*)"RSD PTR ", (CHAR8*)configTable->VendorTable, 8))
			{
				rsdp = (void*)configTable->VendorTable;
			}
		}
		configTable++;
	}

	BootInfo bootInfo;
	bootInfo.frameBuffer = newBuffer;
	bootInfo.font = font;
	bootInfo.memoryMap = EfiMemoryMap;
	bootInfo.mapSize = MapSize;
	bootInfo.descriptorSize = DescriptorSize;
	bootInfo.rsdp = rsdp;

	SystemTable->BootServices->ExitBootServices(ImageHandle, MapKey);

	KernelStart(&bootInfo);

	return EFI_SUCCESS; // Exit the UEFI application
}
