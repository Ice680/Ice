bits 64

%macro pusha64 0
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
%endmacro

%macro popa64 0
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
%endmacro

extern interrupt_handler
int_common_stub:
    test qword [rsp + 24], 0x03
    jz .next0
    swapgs
    .next0:

    pusha64
    mov rdi, rsp
    call interrupt_handler
    popa64
    add rsp, 16

    test qword [rsp + 8], 0x03
    jz .next1
    swapgs
    .next1:

    iretq

%macro isr 1
isr%1:
%if !(%1 == 8 || (%1 >= 10 && %1 <= 14) || %1 == 17 || %1 == 21 || %1 == 29 || %1 == 30)
    push 0
%endif
    push %1
    jmp int_common_stub
%endmacro

%assign i 0
%rep 256
isr i
%assign i i+1
%endrep

section .data
interrupt_table:
%assign i 0
%rep 256
    dq isr%+i
%assign i i+1
%endrep

global interrupt_table