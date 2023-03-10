bits 64

global gdt64_load
global gdt64_32trampoline

gdt64_load:
    lgdt [rdi] ; gdt64_load -> rdi is the first argument passed

    push rbp
    mov rbp, rsp

    push rsi
    push rbp
    pushfq

    push rdx
    push .trampoline

    iretq

.trampoline:
    pop rbp

    ; flush the registers
    mov ss, rsi
    mov gs, rsi
    mov fs, rsi
    mov ds, rsi
    mov es, rsi

    ret
