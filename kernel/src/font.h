#pragma once

struct PSF1_Header {
    unsigned char magic[2]; // Magic bytes for idnetiifcation.
    unsigned char fontMode; // PSF font mode
    unsigned char characterSize; // PSF character size.
};

struct PSF1_FONT {
	PSF1_Header* fontHeader;
	void* glyphBuffer;
};