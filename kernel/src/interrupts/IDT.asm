[GLOBAL default_exception_handler]
[GLOBAL default_interrupt_handler]
[GLOBAL exception_handlers]
[extern ExceptionDump]

default_exception_handler:
    iretq

default_interrupt_handler:
    iretq


%macro make_exception_handler 1
exception%1_handler:
        cli
        push byte 0
        push byte %1
        jmp exception_body
%endmacro

%macro make_error_exception_handler 1
exception%1_handler:
        cli
        push byte %1
        jmp exception_body
%endmacro

make_exception_handler 0
make_exception_handler 1
make_exception_handler 2
make_exception_handler 3
make_exception_handler 4
make_exception_handler 5
make_exception_handler 6
make_exception_handler 7
make_error_exception_handler 8
make_exception_handler 9
make_error_exception_handler 10
make_error_exception_handler 11
make_error_exception_handler 12
make_error_exception_handler 13
make_error_exception_handler 14
make_exception_handler 15
make_exception_handler 16
make_error_exception_handler 17
make_exception_handler 18
make_exception_handler 19

make_exception_handler 32
make_exception_handler 33
make_exception_handler 34
make_exception_handler 35
make_exception_handler 36
make_exception_handler 37
make_exception_handler 38
make_exception_handler 39
make_exception_handler 40
make_exception_handler 41
make_exception_handler 42
make_exception_handler 43
make_exception_handler 44
make_exception_handler 45
make_exception_handler 46
make_exception_handler 47

exception_handlers:
        dq exception0_handler
        dq exception1_handler
        dq exception2_handler
        dq exception3_handler
        dq exception4_handler
        dq exception5_handler
        dq exception6_handler
        dq exception7_handler
        dq exception8_handler
        dq exception9_handler
        dq exception10_handler
        dq exception11_handler
        dq exception12_handler
        dq exception13_handler
        dq exception14_handler
        dq exception15_handler
        dq exception16_handler
        dq exception17_handler
        dq exception18_handler
        dq exception19_handler

        dq exception32_handler
        dq exception33_handler
        dq exception34_handler
        dq exception35_handler
        dq exception36_handler
        dq exception37_handler
        dq exception38_handler
        dq exception39_handler
        dq exception40_handler
        dq exception41_handler
        dq exception42_handler
        dq exception43_handler
        dq exception44_handler
        dq exception45_handler
        dq exception46_handler
        dq exception47_handler

exception_body:
        push rax
        push rcx
        push rdx
        push rbx
        push rbp
        push rsi
        push rdi

        call ExceptionDump

        iretq