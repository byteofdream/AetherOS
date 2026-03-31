#ifndef KERNEL_CONSOLE_H
#define KERNEL_CONSOLE_H

#include "common.h"

enum {
    CONSOLE_COLOR_BLACK = 0x0,
    CONSOLE_COLOR_BLUE = 0x1,
    CONSOLE_COLOR_GREEN = 0x2,
    CONSOLE_COLOR_CYAN = 0x3,
    CONSOLE_COLOR_RED = 0x4,
    CONSOLE_COLOR_MAGENTA = 0x5,
    CONSOLE_COLOR_BROWN = 0x6,
    CONSOLE_COLOR_LIGHT_GREY = 0x7,
    CONSOLE_COLOR_DARK_GREY = 0x8,
    CONSOLE_COLOR_LIGHT_BLUE = 0x9,
    CONSOLE_COLOR_LIGHT_GREEN = 0xA,
    CONSOLE_COLOR_LIGHT_CYAN = 0xB,
    CONSOLE_COLOR_LIGHT_RED = 0xC,
    CONSOLE_COLOR_LIGHT_MAGENTA = 0xD,
    CONSOLE_COLOR_YELLOW = 0xE,
    CONSOLE_COLOR_WHITE = 0xF
};

void console_init(void);
void console_clear(void);
void console_putc(char c);
void console_set_color(uint8_t fg, uint8_t bg);
void console_reset_color(void);
void console_move_to(size_t row, size_t col);
void console_putc_at(size_t row, size_t col, char c);
void console_write_at(size_t row, size_t col, const char *s);
void console_fill_rect(size_t row, size_t col, size_t height, size_t width, char c);
void console_draw_box(size_t row, size_t col, size_t height, size_t width);
void console_write(const char *s);
void console_write_len(const char *s, size_t len);
void console_writeln(const char *s);
void console_write_hex(uint64_t value);
void console_write_dec(uint64_t value);
void console_printf(const char *fmt, ...);

#endif
