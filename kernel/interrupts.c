#include "interrupts.h"
#include "console.h"
#include "pic.h"

static irq_handler_t irq_handlers[16];

static const char *exception_names[] = {
    "Divide Error", "Debug", "NMI", "Breakpoint", "Overflow", "Bound Range",
    "Invalid Opcode", "Device Not Available", "Double Fault", "Coprocessor Segment",
    "Invalid TSS", "Segment Not Present", "Stack Fault", "General Protection",
    "Page Fault", "Reserved", "x87 Fault", "Alignment Check", "Machine Check",
    "SIMD Fault", "Virtualization", "Control Protection"
};

void interrupts_init(void) {
    idt_init();
}

void register_irq_handler(uint8_t irq, irq_handler_t handler) {
    if (irq < 16) {
        irq_handlers[irq] = handler;
    }
}

void interrupt_dispatch(interrupt_frame_t *frame) {
    if (frame->vector < 32) {
        console_write("\nEXCEPTION: ");
        if (frame->vector < ARRAY_SIZE(exception_names)) {
            console_write(exception_names[frame->vector]);
        } else {
            console_write("Unknown");
        }
        console_write(" vector=");
        console_write_dec(frame->vector);
        console_write(" error=");
        console_write_hex(frame->error_code);
        console_write("\n");
        for (;;) {
            __asm__ volatile ("hlt");
        }
    }

    if (frame->vector >= 32 && frame->vector < 48) {
        uint8_t irq = (uint8_t)(frame->vector - 32);
        if (irq_handlers[irq]) {
            irq_handlers[irq](frame);
        }
        pic_send_eoi(irq);
    }
}
