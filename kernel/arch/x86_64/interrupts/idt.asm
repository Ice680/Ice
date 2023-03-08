global load_idt

load_idt:
    lidt [rdi]  ; load idt -> rdi is the first argument
    ret