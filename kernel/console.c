#include "console.h"
#include "port.h"
#include "string.h"
#include <stdarg.h>

static volatile uint16_t *const vga = (uint16_t *)0xB8000;
static size_t cursor_row;
static size_t cursor_col;
static uint8_t color = 0x0f;
static uint8_t default_color = 0x0f;

static inline uint16_t vga_entry(char c, uint8_t attr) {
    return ((uint16_t)attr << 8) | (uint8_t)c;
}

static void scroll_if_needed(void) {
    if (cursor_row < 25) {
        return;
    }

    for (size_t y = 1; y < 25; ++y) {
        for (size_t x = 0; x < 80; ++x) {
            vga[(y - 1) * 80 + x] = vga[y * 80 + x];
        }
    }
    for (size_t x = 0; x < 80; ++x) {
        vga[24 * 80 + x] = vga_entry(' ', color);
    }
    cursor_row = 24;
}

void console_init(void) {
    default_color = (CONSOLE_COLOR_LIGHT_GREY << 4) | CONSOLE_COLOR_WHITE;
    color = default_color;
    console_clear();
}

void console_clear(void) {
    for (size_t i = 0; i < 80 * 25; ++i) {
        vga[i] = vga_entry(' ', color);
    }
    cursor_row = 0;
    cursor_col = 0;
}

void console_set_color(uint8_t fg, uint8_t bg) {
    color = (uint8_t)((bg << 4) | (fg & 0x0F));
}

void console_reset_color(void) {
    color = default_color;
}

void console_move_to(size_t row, size_t col) {
    if (row < 25) {
        cursor_row = row;
    }
    if (col < 80) {
        cursor_col = col;
    }
}

void console_putc_at(size_t row, size_t col, char c) {
    if (row < 25 && col < 80) {
        vga[row * 80 + col] = vga_entry(c, color);
    }
}

void console_write_at(size_t row, size_t col, const char *s) {
    while (*s && row < 25 && col < 80) {
        console_putc_at(row, col++, *s++);
    }
}

void console_fill_rect(size_t row, size_t col, size_t height, size_t width, char c) {
    for (size_t y = 0; y < height && row + y < 25; ++y) {
        for (size_t x = 0; x < width && col + x < 80; ++x) {
            console_putc_at(row + y, col + x, c);
        }
    }
}

void console_draw_box(size_t row, size_t col, size_t height, size_t width) {
    if (height < 2 || width < 2) {
        return;
    }

    console_putc_at(row, col, '+');
    console_putc_at(row, col + width - 1, '+');
    console_putc_at(row + height - 1, col, '+');
    console_putc_at(row + height - 1, col + width - 1, '+');

    for (size_t x = 1; x + 1 < width; ++x) {
        console_putc_at(row, col + x, '-');
        console_putc_at(row + height - 1, col + x, '-');
    }
    for (size_t y = 1; y + 1 < height; ++y) {
        console_putc_at(row + y, col, '|');
        console_putc_at(row + y, col + width - 1, '|');
    }
}

void console_putc(char c) {
    outb(0xE9, (uint8_t)c);
    if (c == '\n') {
        cursor_col = 0;
        cursor_row++;
        scroll_if_needed();
        return;
    }
    if (c == '\r') {
        cursor_col = 0;
        return;
    }
    if (c == '\b') {
        if (cursor_col > 0) {
            cursor_col--;
            vga[cursor_row * 80 + cursor_col] = vga_entry(' ', color);
        }
        return;
    }

    vga[cursor_row * 80 + cursor_col] = vga_entry(c, color);
    cursor_col++;
    if (cursor_col >= 80) {
        cursor_col = 0;
        cursor_row++;
        scroll_if_needed();
    }
}

void console_write(const char *s) {
    console_write_len(s, strlen(s));
}

void console_write_len(const char *s, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        console_putc(s[i]);
    }
}

void console_writeln(const char *s) {
    console_write(s);
    console_putc('\n');
}

void console_write_hex(uint64_t value) {
    static const char digits[] = "0123456789ABCDEF";
    console_write("0x");
    for (int i = 15; i >= 0; --i) {
        console_putc(digits[(value >> (i * 4)) & 0xf]);
    }
}

void console_write_dec(uint64_t value) {
    char buf[32];
    size_t pos = 0;
    if (value == 0) {
        console_putc('0');
        return;
    }
    while (value > 0) {
        buf[pos++] = (char)('0' + (value % 10));
        value /= 10;
    }
    while (pos > 0) {
        console_putc(buf[--pos]);
    }
}

void console_printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    for (size_t i = 0; fmt[i] != '\0'; ++i) {
        if (fmt[i] != '%') {
            console_putc(fmt[i]);
            continue;
        }
        ++i;
        switch (fmt[i]) {
            case 's':
                console_write(va_arg(args, const char *));
                break;
            case 'c':
                console_putc((char)va_arg(args, int));
                break;
            case 'u':
                console_write_dec(va_arg(args, uint64_t));
                break;
            case 'x':
                console_write_hex(va_arg(args, uint64_t));
                break;
            default:
                console_putc('%');
                console_putc(fmt[i]);
                break;
        }
    }
    va_end(args);
}
