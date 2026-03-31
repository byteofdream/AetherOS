#include "idt.h"
#include "string.h"

typedef struct {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
} __attribute__((packed)) idt_entry_t;

typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) idtr_t;

extern void *interrupt_stub_table[];

static idt_entry_t idt[256];

void idt_set_gate(uint8_t vector, void (*handler)(void), uint8_t flags) {
    uint64_t addr = (uint64_t)handler;
    idt[vector].offset_low = addr & 0xffff;
    idt[vector].selector = 0x08;
    idt[vector].ist = 0;
    idt[vector].type_attr = flags;
    idt[vector].offset_mid = (addr >> 16) & 0xffff;
    idt[vector].offset_high = (addr >> 32) & 0xffffffff;
    idt[vector].zero = 0;
}

void idt_load(void) {
    idtr_t idtr = {
        .limit = sizeof(idt) - 1,
        .base = (uint64_t)&idt
    };
    __asm__ volatile ("lidt %0" : : "m"(idtr));
}

void idt_init(void) {
    memset(idt, 0, sizeof(idt));
    for (uint16_t i = 0; i < 48; ++i) {
        idt_set_gate((uint8_t)i, interrupt_stub_table[i], 0x8E);
    }
    idt_load();
}
