#include "ps2.h"
#include "port.h"
#include "keyboard.h"
#include "mouse.h"
#include "pic.h"

static void ps2_wait_write(void) {
    for (int i = 0; i < 100000; ++i) {
        if (!(inb(0x64) & 0x02)) {
            return;
        }
    }
}

static void ps2_wait_read(void) {
    for (int i = 0; i < 100000; ++i) {
        if (inb(0x64) & 0x01) {
            return;
        }
    }
}

static void ps2_cmd(uint8_t cmd) {
    ps2_wait_write();
    outb(0x64, cmd);
}

static void ps2_data(uint8_t data) {
    ps2_wait_write();
    outb(0x60, data);
}

void ps2_init(void) {
    ps2_cmd(0xAD);
    ps2_cmd(0xA7);
    ps2_wait_read();
    (void)inb(0x60);

    ps2_cmd(0x20);
    ps2_wait_read();
    uint8_t config = inb(0x60);
    config |= 0x03;
    config &= ~0x10;
    ps2_cmd(0x60);
    ps2_data(config);

    ps2_cmd(0xAE);
    ps2_cmd(0xA8);

    ps2_cmd(0xD4);
    ps2_data(0xF4);
    ps2_wait_read();
    (void)inb(0x60);

    pic_unmask_irq(1);
    pic_unmask_irq(12);
    keyboard_init();
    mouse_init();
}
