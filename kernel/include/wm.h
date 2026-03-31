#ifndef KERNEL_WM_H
#define KERNEL_WM_H

#include "common.h"
#include "keyboard.h"

void wm_init(void);
int wm_create_window(int x, int y, int width, int height, const char *title);
void wm_recalculate_layout(void);
void wm_remove_window(int index);
void wm_draw(void);
void wm_handle_key(const key_event_t *event);
void wm_task(void *arg);

#endif
