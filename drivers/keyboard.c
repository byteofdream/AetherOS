#include "keyboard.h"
#include "interrupts.h"
#include "port.h"
#include "scheduler.h"

static char queue[128];
static size_t queue_read;
static size_t queue_write;
static bool shift_pressed;
static bool alt_pressed;
static bool ctrl_pressed;
static bool extended_prefix;
static key_event_t event_queue[128];
static size_t event_read;
static size_t event_write;

static const char keymap[128] = {
    [0x02] = '1', [0x03] = '2', [0x04] = '3', [0x05] = '4', [0x06] = '5',
    [0x07] = '6', [0x08] = '7', [0x09] = '8', [0x0A] = '9', [0x0B] = '0',
    [0x0C] = '-', [0x0D] = '=', [0x0E] = '\b', [0x0F] = '\t',
    [0x10] = 'q', [0x11] = 'w', [0x12] = 'e', [0x13] = 'r', [0x14] = 't',
    [0x15] = 'y', [0x16] = 'u', [0x17] = 'i', [0x18] = 'o', [0x19] = 'p',
    [0x1A] = '[', [0x1B] = ']', [0x1C] = '\n',
    [0x1E] = 'a', [0x1F] = 's', [0x20] = 'd', [0x21] = 'f', [0x22] = 'g',
    [0x23] = 'h', [0x24] = 'j', [0x25] = 'k', [0x26] = 'l', [0x27] = ';',
    [0x28] = '\'', [0x29] = '`', [0x2B] = '\\',
    [0x2C] = 'z', [0x2D] = 'x', [0x2E] = 'c', [0x2F] = 'v', [0x30] = 'b',
    [0x31] = 'n', [0x32] = 'm', [0x33] = ',', [0x34] = '.', [0x35] = '/',
    [0x39] = ' '
};

static const char shift_keymap[128] = {
    [0x02] = '!', [0x03] = '@', [0x04] = '#', [0x05] = '$', [0x06] = '%',
    [0x07] = '^', [0x08] = '&', [0x09] = '*', [0x0A] = '(', [0x0B] = ')',
    [0x0C] = '_', [0x0D] = '+', [0x10] = 'Q', [0x11] = 'W', [0x12] = 'E',
    [0x13] = 'R', [0x14] = 'T', [0x15] = 'Y', [0x16] = 'U', [0x17] = 'I',
    [0x18] = 'O', [0x19] = 'P', [0x1A] = '{', [0x1B] = '}', [0x1E] = 'A',
    [0x1F] = 'S', [0x20] = 'D', [0x21] = 'F', [0x22] = 'G', [0x23] = 'H',
    [0x24] = 'J', [0x25] = 'K', [0x26] = 'L', [0x27] = ':', [0x28] = '"',
    [0x29] = '~', [0x2B] = '|', [0x2C] = 'Z', [0x2D] = 'X', [0x2E] = 'C',
    [0x2F] = 'V', [0x30] = 'B', [0x31] = 'N', [0x32] = 'M', [0x33] = '<',
    [0x34] = '>', [0x35] = '?', [0x39] = ' '
};

static void push_char(char c) {
    size_t next = (queue_write + 1) % ARRAY_SIZE(queue);
    if (next != queue_read) {
        queue[queue_write] = c;
        queue_write = next;
    }
}

static void push_event(key_event_t event) {
    size_t next = (event_write + 1) % ARRAY_SIZE(event_queue);
    if (next != event_read) {
        event_queue[event_write] = event;
        event_write = next;
    }
}

static void keyboard_irq(interrupt_frame_t *frame) {
    (void)frame;
    uint8_t code = inb(0x60);

    if (code == 0xE0) {
        extended_prefix = true;
        return;
    }

    if (code == 0x2A || code == 0x36) {
        shift_pressed = true;
        return;
    }
    if (code == 0xAA || code == 0xB6) {
        shift_pressed = false;
        return;
    }
    if (code == 0x38) {
        alt_pressed = true;
        return;
    }
    if (code == 0xB8) {
        alt_pressed = false;
        return;
    }
    if (code == 0x1D) {
        ctrl_pressed = true;
        return;
    }
    if (code == 0x9D) {
        ctrl_pressed = false;
        return;
    }

    bool released = (code & 0x80) != 0;
    uint8_t scancode = code & 0x7F;
    bool extended = extended_prefix;
    extended_prefix = false;

    if (released) {
        return;
    }

    key_event_t event = {
        .type = KEY_NONE,
        .ch = 0,
        .alt = alt_pressed,
        .shift = shift_pressed,
        .ctrl = ctrl_pressed
    };

    if (extended) {
        switch (scancode) {
            case 0x48:
                event.type = KEY_UP;
                break;
            case 0x50:
                event.type = KEY_DOWN;
                break;
            case 0x4B:
                event.type = KEY_LEFT;
                break;
            case 0x4D:
                event.type = KEY_RIGHT;
                break;
            default:
                break;
        }
        if (event.type != KEY_NONE) {
            push_event(event);
        }
        return;
    }

    switch (scancode) {
        case 0x1C:
            event.type = KEY_ENTER;
            event.ch = '\n';
            push_event(event);
            push_char('\n');
            return;
        case 0x0E:
            event.type = KEY_BACKSPACE;
            event.ch = '\b';
            push_event(event);
            push_char('\b');
            return;
        case 0x0F:
            event.type = KEY_TAB;
            event.ch = '\t';
            push_event(event);
            push_char('\t');
            return;
        default:
            break;
    }

    char c = shift_pressed ? shift_keymap[scancode] : keymap[scancode];
    if (c) {
        event.type = KEY_CHAR;
        event.ch = c;
        push_event(event);
        push_char(c);
    }
}

void keyboard_init(void) {
    register_irq_handler(1, keyboard_irq);
}

bool keyboard_get_char(char *out) {
    if (queue_read == queue_write) {
        return false;
    }
    *out = queue[queue_read];
    queue_read = (queue_read + 1) % ARRAY_SIZE(queue);
    return true;
}

bool keyboard_get_event(key_event_t *out) {
    if (event_read == event_write) {
        return false;
    }
    *out = event_queue[event_read];
    event_read = (event_read + 1) % ARRAY_SIZE(event_queue);
    return true;
}

void keyboard_wait_line(char *buffer, size_t capacity) {
    size_t len = 0;
    for (;;) {
        char c;
        if (!keyboard_get_char(&c)) {
            scheduler_yield();
            continue;
        }
        if (c == '\n') {
            buffer[len] = '\0';
            return;
        }
        if (c == '\b') {
            if (len > 0) {
                len--;
            }
            continue;
        }
        if (len + 1 < capacity) {
            buffer[len++] = c;
        }
    }
}
