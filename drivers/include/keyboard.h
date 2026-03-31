#ifndef DRIVERS_KEYBOARD_H
#define DRIVERS_KEYBOARD_H

#include "common.h"

typedef enum {
    KEY_NONE = 0,
    KEY_CHAR,
    KEY_ENTER,
    KEY_BACKSPACE,
    KEY_TAB,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_UP,
    KEY_DOWN
} key_type_t;

typedef struct {
    key_type_t type;
    char ch;
    bool alt;
    bool shift;
    bool ctrl;
} key_event_t;

void keyboard_init(void);
bool keyboard_get_char(char *out);
bool keyboard_get_event(key_event_t *out);
void keyboard_wait_line(char *buffer, size_t capacity);

#endif
