#include "mouse.h"
#include "interrupts.h"
#include "port.h"

static mouse_state_t state;
static uint8_t packet[3];
static size_t packet_index;

static void mouse_irq(interrupt_frame_t *frame) {
    (void)frame;
    packet[packet_index++] = inb(0x60);
    if (packet_index < 3) {
        return;
    }

    packet_index = 0;
    int dx = (int8_t)packet[1];
    int dy = (int8_t)packet[2];
    state.x += dx;
    state.y -= dy;
    state.buttons = packet[0] & 0x07;
}

void mouse_init(void) {
    register_irq_handler(12, mouse_irq);
}

mouse_state_t mouse_state(void) {
    return state;
}
