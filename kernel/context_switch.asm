[BITS 64]

section .text
global context_switch
global task_bootstrap
extern task_boot_entry

; void context_switch(context_t *old_ctx, context_t *new_ctx)
context_switch:
    mov [rdi + 0], rsp
    mov [rdi + 8], rbx
    mov [rdi + 16], rbp
    mov [rdi + 24], r12
    mov [rdi + 32], r13
    mov [rdi + 40], r14
    mov [rdi + 48], r15

    mov rsp, [rsi + 0]
    mov rbx, [rsi + 8]
    mov rbp, [rsi + 16]
    mov r12, [rsi + 24]
    mov r13, [rsi + 32]
    mov r14, [rsi + 40]
    mov r15, [rsi + 48]
    ret

task_bootstrap:
    pop rdi
    pop rsi
    call task_boot_entry
.halt:
    hlt
    jmp .halt
