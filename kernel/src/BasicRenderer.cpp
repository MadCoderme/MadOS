
#include "BasicRenderer.h"

BasicRenderer* GlobalRenderer;

BasicRenderer::BasicRenderer(FrameBuffer* targetFrameBuffer, PSF1_FONT* psfFont) {
    TargetFrameBuffer = targetFrameBuffer;
    Font = psfFont;
    color = 0xffffffff;
    CursorPosition = {0, 0};
}

void BasicRenderer::Clear(uint32_t color)
{
    uint64_t fbBase = (uint64_t)TargetFrameBuffer->BaseAddress;
    uint64_t bytesPerScanline = TargetFrameBuffer->PixelsPerScanline * 4;
    uint64_t fbHeight = TargetFrameBuffer->Height;

    for (int v = 0; v < fbHeight; v++) {
        uint64_t pxlPtrBase = fbBase + (bytesPerScanline * v);
        for (uint32_t* pxlPtr = (uint32_t*)pxlPtrBase; pxlPtr < (uint32_t*)(pxlPtrBase + bytesPerScanline); pxlPtr++) {
            *pxlPtr = color;
        }
    }
}

void BasicRenderer::NextLine()
{
    CursorPosition.x = 0;
    CursorPosition.y += 18;
}

void BasicRenderer::PutChar(char ch, unsigned int xOff, unsigned int yOff) {
    unsigned int* pxlPtr = (unsigned int*)TargetFrameBuffer->BaseAddress;
    char* fontPtr = (char*)Font->glyphBuffer + (ch * Font->fontHeader->characterSize);

    for( unsigned int y = yOff; y < yOff + 16; y++) {
        for (unsigned int x = xOff; x < xOff + 8; x++) {
            if ((*fontPtr & (0b10000000 >> (x - xOff))) > 0) {
                *(unsigned int*)(pxlPtr + x + (y * TargetFrameBuffer->PixelsPerScanline)) = color;
            }
        }

        fontPtr++;
    }  
}

void BasicRenderer::Print(const char* str) {
    char* chr = (char*)str;
    while(*chr != 0) {
        PutChar(*chr, CursorPosition.x, CursorPosition.y);
        CursorPosition.x += 8;

        if(CursorPosition.x + 8 > TargetFrameBuffer->Width) {
            CursorPosition.x = 0;
            CursorPosition.y += 16;
        }
        
        chr++;
    }
}

void BasicRenderer::PutPixel(unsigned int x, unsigned int y) {
    *(unsigned int*)((unsigned int*)TargetFrameBuffer->BaseAddress + x + (y * TargetFrameBuffer->PixelsPerScanline)) = color;
}