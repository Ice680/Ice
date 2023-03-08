bits 64

%macro pusha64 0
    push rax
    push rbx
    push rcx
    push rdx
    push rbp
    push rdi
    push rsi
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
    pop rax
    pop rbx
    pop rcx
    pop rdx
    pop rbp
    pop rdi
    pop rsi
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15
%endmacro

extern isr_handler

global _isr_handler_asm
global _isr_vector_asm

_isr_handler_asm:
    cld ; clear DF on function entry
    pusha64 ; push all

    mov rdi, rsp ; pass the stack pointer as an argument to the c function
    call isr_handler ; call the c function
    mov rsp, rax ; get the new stack pointer returned

    popa64 ; pop all
    add rsp, 16 ; pop error code and int number

    iretq ; return

%macro ISR_NAME 1
dq _ISR%1
%endmacro

%macro ISR_ERR 1
_ISR%1:
        ; error code is being pushed automatically by CPU
    push %1 ; push isr number
    jmp _isr_handler_asm
%endmacro

%macro ISR_NO_ERR 1
_ISR%1:
    push 0 ; push error code by hand as not automatically done by CPU (because "NO_ERR")
    push %1 ; push isr number
    jmp _isr_handler_asm
%endmacro

; exceptions
ISR_NO_ERR 0
ISR_NO_ERR 1
ISR_NO_ERR 2
ISR_NO_ERR 3
ISR_NO_ERR 4
ISR_NO_ERR 5
ISR_NO_ERR 6
ISR_NO_ERR 7
ISR_ERR	8
ISR_NO_ERR 9
ISR_ERR	10
ISR_ERR 11
ISR_ERR 12
ISR_ERR 13
ISR_ERR 14
ISR_NO_ERR 15
ISR_NO_ERR 16
ISR_ERR	17
ISR_NO_ERR 18
ISR_NO_ERR 19
ISR_NO_ERR 20
ISR_ERR	21
ISR_NO_ERR 22
ISR_NO_ERR 23
ISR_NO_ERR 24
ISR_NO_ERR 25
ISR_NO_ERR 26
ISR_NO_ERR 27
ISR_NO_ERR 28
ISR_NO_ERR 29
ISR_NO_ERR 30
ISR_NO_ERR 31

; standard ISA IRQ's
ISR_NO_ERR 32
ISR_NO_ERR 33
ISR_NO_ERR 34
ISR_NO_ERR 35
ISR_NO_ERR 36
ISR_NO_ERR 37
ISR_NO_ERR 38
ISR_NO_ERR 39
ISR_NO_ERR 40
ISR_NO_ERR 41
ISR_NO_ERR 42
ISR_NO_ERR 43
ISR_NO_ERR 44
ISR_NO_ERR 45
ISR_NO_ERR 46
ISR_NO_ERR 47

; remaining IRQs
%assign i 48
%rep 256
    ISR_NO_ERR i
%assign i i+1
%endrep

_isr_vector_asm:
%assign i 0
%rep 256
    ISR_NAME i
%assign i i+1
%endrep

