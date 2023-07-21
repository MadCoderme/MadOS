#include "keyboard.h"


const char ASCIITable[] = {
         0 ,  0 , '1', '2',
        '3', '4', '5', '6',
        '7', '8', '9', '0',
        '-', '=',  0 ,  0 ,
        'q', 'w', 'e', 'r',
        't', 'y', 'u', 'i',
        'o', 'p', '[', ']',
         0 ,  0 , 'a', 's',
        'd', 'f', 'g', 'h',
        'j', 'k', 'l', ';',
        '\'','`',  0 , '\\',
        'z', 'x', 'c', 'v',
        'b', 'n', 'm', ',',
        '.', '/',  0 , '*',
         0 , ' '
};

bool upperCase = false;

void TranslateScancode(uint8_t byte)
{
    if (byte > 58)
    {
        // handle key release
    } 
    else
    {
        char c = ASCIITable[byte];

        if (upperCase)
            c = ASCIITable[byte] - 32;

        GlobalRenderer->PutChar(c);
    }
}

void HandleKeyboard()
{
    uint8_t scancode = inb(0x60);

    switch (scancode)
    {
    case 0xAA:
        upperCase = false;
        break;
    case 0x2A:
        upperCase = true;
        break;
    case 0xf: 
      for (int i = 0; i < 4; i++) GlobalRenderer->PutChar(' ');
      break;
    case 0x1C:
        GlobalRenderer->NextLine();
        break;
    default:
        TranslateScancode(scancode);
        break;
    }
}