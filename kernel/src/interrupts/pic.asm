RemapPIC:
    mov al, 0x11
    out 0x20, al     ;restart PIC1
    out 0xA0, al     ;restart PIC2

    mov al, 0x20
    out 0x21, al     ;PIC1 now starts at 32
    mov al, 0x28
    out 0xA1, al     ;PIC2 now starts at 40

    mov al, 0x04
    out 0x21, al     ;setup cascading
    mov al, 0x02
    out 0xA1, al

    mov al, 0x01
    out 0x21, al
    out 0xA1, al     ;done!

    mov al, 0x00
    out 0x21, al
    out 0xA1, al

    retq

Global RemapPIC