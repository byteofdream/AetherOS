#include "pit.h"
#include "interrupts.h"
#include "port.h"
#include "scheduler.h"

static volatile uint64_t ticks;

static void pit_irq(interrupt_frame_t *frame) {
    (void)frame;
    ticks++;
}

void pit_init(uint32_t frequency) {
    uint16_t divisor = (uint16_t)(1193182 / frequency);
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xff);
    outb(0x40, divisor >> 8);
    register_irq_handler(0, pit_irq);
}

uint64_t pit_ticks(void) {
    return ticks;
}

void pit_sleep(uint64_t wait_ticks) {
    uint64_t start = ticks;
    while ((ticks - start) < wait_ticks) {
        scheduler_yield();
    }
}
