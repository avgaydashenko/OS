.code64
.section .text
.globl start_thread
.globl switch_threads

start_thread:
        callq check_thread_finished
        callq end_critical_section

        popq %rax
        popq %rdi

        callq *%rax

        movq %rax, %rdi
        callq finish_current_thread
        loop: jmp loop

switch_threads:
        pushq %rbp
        pushq %rbx
        pushq %r12
        pushq %r13
        pushq %r14
        pushq %r15

        movq %rsp, (%rdi)
        movq %rsi, %rsp

        popq %r15
        popq %r14
        popq %r13
        popq %r12
        popq %rbx
        popq %rbp

        ret