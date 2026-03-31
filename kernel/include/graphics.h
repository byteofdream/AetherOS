#ifndef KERNEL_GRAPHICS_H
#define KERNEL_GRAPHICS_H

#include "common.h"
#include "multiboot.h"

typedef uint32_t gfx_color_t;

void graphics_init(const boot_info_t *boot_info);
bool graphics_available(void);
uint32_t graphics_width(void);
uint32_t graphics_height(void);
void graphics_clear(gfx_color_t color);
void draw_pixel(int x, int y, gfx_color_t color);
void draw_rect(int x, int y, int width, int height, gfx_color_t color);
void draw_border(int x, int y, int width, int height, gfx_color_t color);
void draw_rect_vertical_gradient(int x, int y, int width, int height, gfx_color_t top, gfx_color_t bottom);
void draw_text(int x, int y, const char *text, gfx_color_t color);
void draw_text_scaled(int x, int y, const char *text, gfx_color_t color, int scale);
void graphics_present(void);
const char* graphics_get_gpu_vendor(void);
const char* graphics_get_gpu_model(void);
#endif