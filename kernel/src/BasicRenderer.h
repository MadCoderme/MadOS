#pragma once
#include "math.h"
#include "FrameBuffer.h"
#include "font.h"


class BasicRenderer {
    public:
    Point CursorPosition;
    FrameBuffer* TargetFrameBuffer;
    PSF1_FONT* Font;
    unsigned int color;
    void Initialize(FrameBuffer* targetFrameBuffer, PSF1_FONT* font);
    void PutChar(char ch, unsigned int xOff, unsigned int yOff);
    void Print(const char* str);
    void PutPixel(unsigned int x, unsigned int y);
};

extern BasicRenderer GlobalRenderer;