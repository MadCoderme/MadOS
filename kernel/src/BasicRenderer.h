#pragma once
#include "math.h"
#include "FrameBuffer.h"
#include "font.h"
#include <stdint.h>

class BasicRenderer {
    public:
    BasicRenderer(FrameBuffer* targetFrameBuffer, PSF1_FONT* psfFont);
    Point CursorPosition;
    FrameBuffer* TargetFrameBuffer;
    PSF1_FONT* Font;
    unsigned int color;
    void PutChar(char ch);
    void Print(const char* str);
    void PutPixel(unsigned int x, unsigned int y);
    void Clear(uint32_t color);
    void NextLine();
};

extern BasicRenderer* GlobalRenderer;