    .code64
    .global handler_empty, handler_pop, handler_pit
    .extern pit_handler
        
handler_empty:
    iretq

handler_pop:
    add $8, %rsp
    iretq

handler_pit:
    push %rax
    push %rcx
    push %rsi
    push %rdi
    push %r8
    push %r9
    push %r10
    push %r11

    call pit_handler
    
    pop %r11
    pop %r10
    pop %r9
    pop %r8
    pop %rdi
    pop %rsi
    pop %rcx
    pop %rax
    
    iretq
