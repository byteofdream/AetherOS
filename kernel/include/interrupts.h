#ifndef KERNEL_INTERRUPTS_H
#define KERNEL_INTERRUPTS_H

#include "idt.h"

typedef void (*irq_handler_t)(interrupt_frame_t *frame);

void interrupts_init(void);
void register_irq_handler(uint8_t irq, irq_handler_t handler);
void interrupt_dispatch(interrupt_frame_t *frame);

#endif
