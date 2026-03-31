[BITS 64]

section .text
global interrupt_stub_table
extern interrupt_dispatch

%macro PUSH_REGS 0
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

%macro POP_REGS 0
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rsi
    pop rdi
    pop rbp
    pop rdx
    pop rcx
    pop rbx
    pop rax
%endmacro

%macro ISR_NOERR 1
global isr_stub_%1
isr_stub_%1:
    push 0
    push %1
    PUSH_REGS
    mov rdi, rsp
    call interrupt_dispatch
    POP_REGS
    add rsp, 16
    iretq
%endmacro

%macro ISR_ERR 1
global isr_stub_%1
isr_stub_%1:
    push %1
    PUSH_REGS
    mov rdi, rsp
    call interrupt_dispatch
    POP_REGS
    add rsp, 8
    iretq
%endmacro

ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR   8
ISR_NOERR 9
ISR_ERR   10
ISR_ERR   11
ISR_ERR   12
ISR_ERR   13
ISR_ERR   14
ISR_NOERR 15
ISR_NOERR 16
ISR_ERR   17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_ERR   21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_ERR   30
ISR_NOERR 31

%assign i 32
%rep 16
ISR_NOERR i
%assign i i+1
%endrep

interrupt_stub_table:
    dq isr_stub_0, isr_stub_1, isr_stub_2, isr_stub_3
    dq isr_stub_4, isr_stub_5, isr_stub_6, isr_stub_7
    dq isr_stub_8, isr_stub_9, isr_stub_10, isr_stub_11
    dq isr_stub_12, isr_stub_13, isr_stub_14, isr_stub_15
    dq isr_stub_16, isr_stub_17, isr_stub_18, isr_stub_19
    dq isr_stub_20, isr_stub_21, isr_stub_22, isr_stub_23
    dq isr_stub_24, isr_stub_25, isr_stub_26, isr_stub_27
    dq isr_stub_28, isr_stub_29, isr_stub_30, isr_stub_31
    dq isr_stub_32, isr_stub_33, isr_stub_34, isr_stub_35
    dq isr_stub_36, isr_stub_37, isr_stub_38, isr_stub_39
    dq isr_stub_40, isr_stub_41, isr_stub_42, isr_stub_43
    dq isr_stub_44, isr_stub_45, isr_stub_46, isr_stub_47
