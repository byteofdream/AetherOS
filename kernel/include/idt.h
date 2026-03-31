#ifndef KERNEL_IDT_H
#define KERNEL_IDT_H

#include "common.h"

typedef struct interrupt_frame {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rsi, rdi, rbp, rdx, rcx, rbx, rax;
    uint64_t vector;
    uint64_t error_code;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
} interrupt_frame_t;

void idt_init(void);
void idt_set_gate(uint8_t vector, void (*handler)(void), uint8_t flags);
void idt_load(void);

#endif
