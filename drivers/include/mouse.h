#ifndef DRIVERS_MOUSE_H
#define DRIVERS_MOUSE_H

#include "common.h"

typedef struct {
    int x;
    int y;
    uint8_t buttons;
} mouse_state_t;

void mouse_init(void);
mouse_state_t mouse_state(void);

#endif
