#include "wm.h"
#include "graphics.h"
#include "memory.h"
#include "pit.h"
#include "ramfs.h"
#include "scheduler.h"
#include "string.h"

typedef enum {
    WM_WINDOW_TERMINAL = 0,
    WM_WINDOW_APP = 1
} wm_window_kind_t;

typedef struct {
    bool used;
    bool active;
    int x;
    int y;
    int width;
    int height;
    int target_x;
    int target_y;
    int target_width;
    int target_height;
    char title[32];
    wm_window_kind_t kind;
    char lines[24][96];
    size_t line_count;
    char input[96];
    size_t input_len;
} wm_window_t;

typedef struct {
    gfx_color_t background;
    gfx_color_t background_alt;
    gfx_color_t active_window;
    gfx_color_t inactive_window;
    gfx_color_t active_window_alt;
    gfx_color_t inactive_window_alt;
    gfx_color_t border;
    gfx_color_t text;
    gfx_color_t text_dim;
    gfx_color_t active_title;
    gfx_color_t inactive_title;
    gfx_color_t bar_bg;
    gfx_color_t bar_bg_alt;
    gfx_color_t bar_text;
    gfx_color_t shadow;
    gfx_color_t accent;
    gfx_color_t accent_soft;
} wm_theme_t;

enum {
    WM_FONT_W = 8,
    WM_FONT_H = 8,
    WM_BODY_SCALE = 1,
    WM_TITLE_SCALE = 1,
    WM_BAR_SCALE = 1,
    WM_BRAND_SCALE = 2,
    WM_BAR_H = 30,
    WM_TITLE_H = 24,
    WM_OUTER_GAP = 12,
    WM_INNER_GAP = 12,
    WM_SHADOW = 4
};

static wm_window_t windows[6];
static size_t window_count;
static int active_window = -1;
static fs_node_t *terminal_cwd;
static bool wm_dirty = true;
static uint64_t wm_last_clock_second;
static uint64_t wm_last_anim_tick;
static uint64_t wm_flash_until = 0;
static int wm_flash_window = -1;
static const wm_theme_t theme = {
    .background = 0x0B1220,
    .background_alt = 0x101A2E,
    .active_window = 0x162238,
    .active_window_alt = 0x1B2D49,
    .inactive_window = 0x111827,
    .inactive_window_alt = 0x151D2C,
    .border = 0x385071,
    .text = 0xE5EEF9,
    .text_dim = 0x9AB0C8,
    .active_title = 0xF6C85F,
    .inactive_title = 0x9FB3C8,
    .bar_bg = 0x0F172A,
    .bar_bg_alt = 0x162238,
    .bar_text = 0xDCE7F5,
    .shadow = 0x060A12,
    .accent = 0x63C7FF,
    .accent_soft = 0x2E7CB8
};

static void wm_request_redraw(void) {
    wm_dirty = true;
}

static void wm_flash_window_effect(int index) {
    wm_flash_window = index;
    wm_flash_until = pit_ticks() + 20;
}

static int scaled_text_width_px(int chars, int scale) {
    return chars * WM_FONT_W * scale;
}

static void clipped_text_scaled(int x, int y, int width_px, const char *text, gfx_color_t color, int scale) {
    char buffer[128];
    size_t max_chars = (size_t)(width_px / (WM_FONT_W * scale));
    size_t i = 0;

    if (width_px <= 0) {
        return;
    }

    while (text[i] != '\0' && i < max_chars && i + 1 < sizeof(buffer)) {
        buffer[i] = text[i];
        ++i;
    }
    buffer[i] = '\0';
    draw_text_scaled(x, y, buffer, color, scale);
}

static size_t used_window_indices(int *indices, size_t size) {
    size_t count = 0;
    for (size_t i = 0; i < ARRAY_SIZE(windows) && count < size; ++i) {
        if (windows[i].used) {
            indices[count++] = (int)i;
        }
    }
    return count;
}

static void wm_set_active(int index) {
    active_window = index;
    for (size_t i = 0; i < ARRAY_SIZE(windows); ++i) {
        windows[i].active = (int)i == index && windows[i].used;
    }
    wm_flash_window_effect(index);
    wm_request_redraw();
}

static void window_add_line(wm_window_t *window, const char *text) {
    if (!window) {
        return;
    }

    if (window->line_count == ARRAY_SIZE(window->lines)) {
        for (size_t i = 1; i < window->line_count; ++i) {
            memcpy(window->lines[i - 1], window->lines[i], sizeof(window->lines[i]));
        }
        window->line_count--;
    }

    strncpy(window->lines[window->line_count], text, sizeof(window->lines[window->line_count]) - 1);
    window->lines[window->line_count][sizeof(window->lines[window->line_count]) - 1] = '\0';
    window->line_count++;
    wm_request_redraw();
}

static void window_add_wrapped_text(wm_window_t *window, const char *text) {
    char line[96];
    size_t pos = 0;

    for (size_t i = 0;; ++i) {
        char c = text[i];
        if (c == '\n' || c == '\0') {
            line[pos] = '\0';
            window_add_line(window, line);
            pos = 0;
            if (c == '\0') {
                break;
            }
        } else if (pos + 1 < sizeof(line)) {
            line[pos++] = c;
        }
    }
}

static void format_uptime(char *buffer, size_t size) {
    uint64_t seconds = pit_ticks() / 100;
    uint64_t hours = seconds / 3600;
    uint64_t minutes = (seconds / 60) % 60;
    uint64_t secs = seconds % 60;

    if (size < 9) {
        if (size > 0) {
            buffer[0] = '\0';
        }
        return;
    }

    buffer[0] = (char)('0' + ((hours / 10) % 10));
    buffer[1] = (char)('0' + (hours % 10));
    buffer[2] = ':';
    buffer[3] = (char)('0' + ((minutes / 10) % 10));
    buffer[4] = (char)('0' + (minutes % 10));
    buffer[5] = ':';
    buffer[6] = (char)('0' + ((secs / 10) % 10));
    buffer[7] = (char)('0' + (secs % 10));
    buffer[8] = '\0';
}

static void draw_bar(void) {
    char clock[16];
    char count[16];
    const char *title = (active_window >= 0 && windows[active_window].used) ? windows[active_window].title : "No Window";
    uint32_t screen_w = graphics_width();

    draw_rect_vertical_gradient(0, 0, (int)screen_w, WM_BAR_H, theme.bar_bg_alt, theme.bar_bg);
    draw_rect(0, WM_BAR_H - 2, (int)screen_w, 1, theme.accent_soft);
    draw_rect(0, WM_BAR_H - 1, (int)screen_w, 1, theme.border);
    draw_text_scaled(14, 4, "AetherOS", theme.bar_text, WM_BRAND_SCALE);
    draw_rect(146, 6, 2, WM_BAR_H - 12, theme.accent_soft);
    draw_rect(160, 7, (int)screen_w - 356, WM_BAR_H - 14, 0x122035);
    draw_border(160, 7, (int)screen_w - 356, WM_BAR_H - 14, 0x233854);
    clipped_text_scaled(172, 10, (int)screen_w - 380, title, theme.active_title, WM_BAR_SCALE);

    count[0] = '#';
    count[1] = (char)('0' + (window_count % 10));
    count[2] = '\0';
    format_uptime(clock, sizeof(clock));
    draw_text_scaled((int)screen_w - 190, 10, "windows", theme.text_dim, WM_BAR_SCALE);
    draw_text_scaled((int)screen_w - 88, 10, count, theme.bar_text, WM_BAR_SCALE);
    draw_text_scaled((int)screen_w - 148, 10, clock, theme.bar_text, WM_BAR_SCALE);
}

static void draw_shadow(const wm_window_t *window) {
    draw_rect(window->x + window->width, window->y + WM_SHADOW, WM_SHADOW, window->height, theme.shadow);
    draw_rect(window->x + WM_SHADOW, window->y + window->height, window->width, WM_SHADOW, theme.shadow);
}

static void draw_window(const wm_window_t *window) {
    int window_index = (int)(window - &windows[0]);
    bool is_flashing = (wm_flash_window == window_index && pit_ticks() < wm_flash_until);
    bool flash_on = (pit_ticks() / 10) % 2 == 0;
    
    gfx_color_t title_color = window->active ? theme.active_title : theme.inactive_title;
    gfx_color_t body_color = window->active ? theme.active_window : theme.inactive_window;
    gfx_color_t body_color_alt = window->active ? theme.active_window_alt : theme.inactive_window_alt;
    gfx_color_t border_color = window->active ? theme.accent : theme.border;
    
    if (is_flashing && flash_on) {
        title_color = 0xFFFFFF;
        border_color = 0xFFFF00;
    }
    int x = window->x;
    int y = window->y;
    int width = window->width;
    int height = window->height;
    int text_x = x + 14;
    int text_y = y + WM_TITLE_H + 10;
    int line_step = WM_FONT_H * WM_BODY_SCALE + 4;
    int visible_lines = (height - WM_TITLE_H - 28) / line_step;

    if (width <= 0 || height <= 0) {
        return;
    }

    draw_shadow(window);
    draw_rect_vertical_gradient(x, y, width, height, body_color_alt, body_color);
    draw_rect_vertical_gradient(x, y, width, WM_TITLE_H,
                                window->active ? 0x28456C : 0x1B2434,
                                window->active ? 0x1D324F : 0x161E2B);
    draw_border(x, y, width, height, border_color);
    draw_border(x + 2, y + 2, width - 4, height - 4, window->active ? 0x243C5E : 0x1A2638);
    draw_rect(x + 1, y + 1, width - 2, 1, window->active ? 0x6FD4FF : 0x41566F);
    draw_rect(x + 1, y + WM_TITLE_H, width - 2, 1, border_color);
    draw_rect(x + 10, y + WM_TITLE_H - 4, width - 20, 1, theme.accent_soft);
    draw_rect(x + 10, y + 8, 7, 7, window->active ? theme.active_title : theme.text_dim);
    if (window->active) {
        draw_rect(x + width - 28, y + 8, 16, 2, theme.accent);
        draw_rect(x + width - 28, y + 12, 10, 2, theme.accent_soft);
    }
    clipped_text_scaled(x + 26, y + 7, width - 38, window->title, title_color, WM_TITLE_SCALE);

    if (visible_lines < 1) {
        return;
    }

    size_t start = 0;
    if ((int)window->line_count > visible_lines) {
        start = window->line_count - (size_t)visible_lines;
    }

    for (size_t i = start; i < window->line_count && (int)(i - start) < visible_lines; ++i) {
        clipped_text_scaled(text_x, text_y + (int)(i - start) * line_step, width - 28,
                     window->lines[i],
                     i + 1 == window->line_count ? theme.text : theme.text_dim,
                     WM_BODY_SCALE);
    }

    if (window->kind == WM_WINDOW_TERMINAL) {
        int input_y = y + height - (WM_FONT_H * WM_BODY_SCALE) - 14;
        draw_rect(x + 10, input_y - 6, width - 20, WM_FONT_H * WM_BODY_SCALE + 12,
                  window->active ? 0x0F1B2C : 0x0D1522);
        draw_border(x + 10, input_y - 6, width - 20, WM_FONT_H * WM_BODY_SCALE + 12,
                    window->active ? theme.accent_soft : theme.border);
        draw_text_scaled(text_x, input_y, ">", theme.active_title, WM_BODY_SCALE);
        clipped_text_scaled(text_x + scaled_text_width_px(2, WM_BODY_SCALE), input_y, width - 42, window->input, theme.text, WM_BODY_SCALE);
        if (window->active && ((pit_ticks() / 35) % 2 == 0)) {
            int cursor_x = text_x + scaled_text_width_px((int)window->input_len + 2, WM_BODY_SCALE);
            draw_rect(cursor_x, input_y, 6, WM_FONT_H * WM_BODY_SCALE, theme.active_title);
        }
    }
}

static int step_towards(int current, int target) {
    int delta = target - current;
    if (delta == 0) {
        return current;
    }
    if (delta > 0) {
        int step = delta / 5;
        if (step < 1) {
            step = 1;
        }
        if (step > 24) {
            step = 24;
        }
        return current + step;
    }
    int step = (-delta) / 5;
    if (step < 1) {
        step = 1;
    }
    if (step > 24) {
        step = 24;
    }
    return current - step;
}

static bool wm_animate_layout(void) {
    bool changed = false;
    for (size_t i = 0; i < ARRAY_SIZE(windows); ++i) {
        if (!windows[i].used) {
            continue;
        }
        int nx = step_towards(windows[i].x, windows[i].target_x);
        int ny = step_towards(windows[i].y, windows[i].target_y);
        int nw = step_towards(windows[i].width, windows[i].target_width);
        int nh = step_towards(windows[i].height, windows[i].target_height);
        if (nx != windows[i].x || ny != windows[i].y || nw != windows[i].width || nh != windows[i].height) {
            windows[i].x = nx;
            windows[i].y = ny;
            windows[i].width = nw;
            windows[i].height = nh;
            changed = true;
        }
    }
    if (changed) {
        wm_request_redraw();
    }
    return changed;
}

void wm_draw(void) {
    if (!graphics_available() || !wm_dirty) {
        return;
    }

    graphics_clear(theme.background);
    draw_rect_vertical_gradient(0, 0, (int)graphics_width(), (int)graphics_height(), theme.background_alt, theme.background);
    draw_rect(0, (int)graphics_height() - 40, (int)graphics_width(), 40, 0x0A1020);
    draw_bar();
    for (size_t i = 0; i < ARRAY_SIZE(windows); ++i) {
        if (windows[i].used) {
            draw_window(&windows[i]);
        }
    }
    graphics_present();
    wm_dirty = false;
}

int wm_create_window(int x, int y, int width, int height, const char *title) {
    (void)x;
    (void)y;
    (void)width;
    (void)height;

    for (size_t i = 0; i < ARRAY_SIZE(windows); ++i) {
        if (!windows[i].used) {
            memset(&windows[i], 0, sizeof(windows[i]));
            windows[i].used = true;
            strncpy(windows[i].title, title, sizeof(windows[i].title) - 1);
            window_count++;
            wm_set_active((int)i);
            wm_recalculate_layout();
            return (int)i;
        }
    }
    return -1;
}

void wm_recalculate_layout(void) {
    int indices[ARRAY_SIZE(windows)];
    size_t count = used_window_indices(indices, ARRAY_SIZE(indices));
    int area_x = WM_OUTER_GAP;
    int area_y = WM_BAR_H + WM_OUTER_GAP;
    int area_w = (int)graphics_width() - (WM_OUTER_GAP * 2) - WM_SHADOW;
    int area_h = (int)graphics_height() - WM_BAR_H - (WM_OUTER_GAP * 2) - WM_SHADOW;

    if (!graphics_available()) {
        return;
    }
    if (count == 0) {
        wm_request_redraw();
        return;
    }

    if (count == 1) {
        wm_window_t *window = &windows[indices[0]];
        window->target_x = area_x;
        window->target_y = area_y;
        window->target_width = area_w;
        window->target_height = area_h;
        if (window->width == 0 || window->height == 0) {
            window->x = area_x + 24;
            window->y = area_y + 16;
            window->width = area_w - 48;
            window->height = area_h - 32;
        }
        wm_request_redraw();
        return;
    }

    if (count == 2) {
        int split_w = (area_w - WM_INNER_GAP) / 2;
        windows[indices[0]].target_x = area_x;
        windows[indices[0]].target_y = area_y;
        windows[indices[0]].target_width = split_w;
        windows[indices[0]].target_height = area_h;

        windows[indices[1]].target_x = area_x + split_w + WM_INNER_GAP;
        windows[indices[1]].target_y = area_y;
        windows[indices[1]].target_width = area_w - split_w - WM_INNER_GAP;
        windows[indices[1]].target_height = area_h;
        for (int k = 0; k < 2; ++k) {
            wm_window_t *window = &windows[indices[k]];
            if (window->width == 0 || window->height == 0) {
                window->x = window->target_x + 20;
                window->y = window->target_y + 12;
                window->width = window->target_width - 40;
                window->height = window->target_height - 24;
            }
        }
        wm_request_redraw();
        return;
    }

    size_t cols = 1;
    while (cols * cols < count) {
        cols++;
    }
    size_t rows = (count + cols - 1) / cols;
    int content_w = area_w - (int)((cols - 1) * WM_INNER_GAP);
    int content_h = area_h - (int)((rows - 1) * WM_INNER_GAP);
    int base_w = content_w / (int)cols;
    int extra_w = content_w % (int)cols;
    int base_h = content_h / (int)rows;
    int extra_h = content_h % (int)rows;
    size_t idx = 0;
    int y_cursor = area_y;

    for (size_t row = 0; row < rows; ++row) {
        int row_h = base_h + ((int)row < extra_h ? 1 : 0);
        int x_cursor = area_x;
        for (size_t col = 0; col < cols && idx < count; ++col) {
            int col_w = base_w + ((int)col < extra_w ? 1 : 0);
            wm_window_t *window = &windows[indices[idx++]];
            window->target_x = x_cursor;
            window->target_y = y_cursor;
            window->target_width = col_w;
            window->target_height = row_h;
            if (window->width == 0 || window->height == 0) {
                window->x = x_cursor + 18;
                window->y = y_cursor + 12;
                window->width = col_w - 36;
                window->height = row_h - 24;
            }
            x_cursor += col_w + WM_INNER_GAP;
        }
        y_cursor += row_h + WM_INNER_GAP;
    }
    wm_request_redraw();
}

static wm_window_t *active_terminal(void) {
    if (active_window < 0) {
        return 0;
    }
    if (!windows[active_window].used || windows[active_window].kind != WM_WINDOW_TERMINAL) {
        return 0;
    }
    return &windows[active_window];
}

static void terminal_prompt_path(char *buffer, size_t size) {
    ramfs_path(terminal_cwd, buffer, size);
}

static void create_app_window(const char *name, const char *content) {
    int index = wm_create_window(0, 0, 0, 0, name);
    if (index < 0) {
        wm_window_t *term = active_terminal();
        if (term) {
            window_add_line(term, "wm: no free window slots");
        }
        return;
    }

    windows[index].kind = WM_WINDOW_APP;
    window_add_wrapped_text(&windows[index], content);
}

static void terminal_run_command(wm_window_t *window) {
    char line[96];
    strncpy(line, window->input, sizeof(line) - 1);
    line[sizeof(line) - 1] = '\0';
    window->input_len = 0;
    window->input[0] = '\0';

    char prompt[96];
    char echoed[96];
    terminal_prompt_path(prompt, sizeof(prompt));
    strncpy(echoed, prompt, sizeof(echoed) - 1);
    echoed[sizeof(echoed) - 1] = '\0';

    size_t len = strlen(echoed);
    if (len + 2 < sizeof(echoed)) {
        echoed[len++] = '>';
        echoed[len++] = ' ';
        echoed[len] = '\0';
    }
    for (size_t i = 0; line[i] != '\0' && len + 1 < sizeof(echoed); ++i) {
        echoed[len++] = line[i];
        echoed[len] = '\0';
    }
    window_add_line(window, echoed);

    char *cmd = line;
    while (*cmd == ' ') {
        ++cmd;
    }
    char *arg = strchr(cmd, ' ');
    if (arg) {
        *arg++ = '\0';
        while (*arg == ' ') {
            ++arg;
        }
    } else {
        arg = "";
    }

    if (strcmp(cmd, "") == 0) {
    } else if (strcmp(cmd, "help") == 0) {
        window_add_line(window, "help, ls, cd, pwd, cat, run, clear, mem, uptime");
        window_add_line(window, "sysinfo, cpuinfo, gpu, osver, shutdown, reboot");
    } else if (strcmp(cmd, "ls") == 0) {
        for (size_t i = 0; i < terminal_cwd->child_count; ++i) {
            char entry[96];
            strncpy(entry, terminal_cwd->children[i]->name, sizeof(entry) - 2);
            entry[sizeof(entry) - 2] = '\0';
            if (terminal_cwd->children[i]->is_dir) {
                size_t e = strlen(entry);
                entry[e] = '/';
                entry[e + 1] = '\0';
            }
            window_add_line(window, entry);
        }
    } else if (strcmp(cmd, "pwd") == 0) {
        char path[96];
        terminal_prompt_path(path, sizeof(path));
        window_add_line(window, path);
    } else if (strcmp(cmd, "cd") == 0) {
        fs_node_t *node = ramfs_resolve(terminal_cwd, arg);
        if (!node || !node->is_dir) {
            window_add_line(window, "cd: no such directory");
        } else {
            terminal_cwd = node;
        }
    } else if (strcmp(cmd, "cat") == 0) {
        fs_node_t *node = ramfs_resolve(terminal_cwd, arg);
        if (!node || node->is_dir) {
            window_add_line(window, "cat: file not found");
        } else {
            char buffer[96];
            size_t pos = 0;
            for (size_t i = 0; i < node->size; ++i) {
                char c = (char)node->data[i];
                if (c == '\n' || pos + 1 == sizeof(buffer)) {
                    buffer[pos] = '\0';
                    window_add_line(window, buffer);
                    pos = 0;
                    if (c != '\n' && pos + 1 < sizeof(buffer)) {
                        buffer[pos++] = c;
                    }
                } else {
                    buffer[pos++] = c;
                }
            }
            if (pos > 0) {
                buffer[pos] = '\0';
                window_add_line(window, buffer);
            }
        }
    } else if (strcmp(cmd, "clear") == 0) {
        window->line_count = 0;
        wm_request_redraw();
    } else if (strcmp(cmd, "mem") == 0) {
        char buffer[96];
        strcpy(buffer, "Memory MB: ");
        char digits[16];
        size_t d = 0;
        uint64_t mem = memory_total_bytes() / (1024 * 1024);
        if (mem == 0) {
            digits[d++] = '0';
        }
        while (mem > 0) {
            digits[d++] = (char)('0' + (mem % 10));
            mem /= 10;
        }
        while (d > 0 && strlen(buffer) + 1 < sizeof(buffer)) {
            size_t b = strlen(buffer);
            buffer[b] = digits[--d];
            buffer[b + 1] = '\0';
        }
        window_add_line(window, buffer);
    } else if (strcmp(cmd, "uptime") == 0) {
        char buffer[96];
        strcpy(buffer, "Ticks: ");
        char digits[24];
        size_t d = 0;
        uint64_t ticks = pit_ticks();
        if (ticks == 0) {
            digits[d++] = '0';
        }
        while (ticks > 0) {
            digits[d++] = (char)('0' + (ticks % 10));
            ticks /= 10;
        }
        while (d > 0 && strlen(buffer) + 1 < sizeof(buffer)) {
            size_t b = strlen(buffer);
            buffer[b] = digits[--d];
            buffer[b + 1] = '\0';
        }
        window_add_line(window, buffer);
    } else if (strcmp(cmd, "osver") == 0) {
        window_add_line(window, "AetherOS v1.0");
        window_add_line(window, "Build: 2026");
        window_add_line(window, "Kernel: x86 with graphics WM");
    } else if (strcmp(cmd, "sysinfo") == 0) {
        window_add_line(window, "AetherOS System Information");
        window_add_line(window, "----------------------------");
        window_add_line(window, "OS Name: AetherOS");
        window_add_line(window, "Architecture: x86_64");
        window_add_line(window, "Bootloader: GRUB2 (Multiboot2)");
        window_add_line(window, "Mode: Long Mode (64-bit)");
        window_add_line(window, "Kernel Features:");
        window_add_line(window, "  [+] GDT/IDT/PIC/PIT");
        window_add_line(window, "  [+] Process Scheduler");
        window_add_line(window, "  [+] Memory Management");
        window_add_line(window, "  [+] RAMFS Filesystem");
        window_add_line(window, "  [+] Window Manager");
        window_add_line(window, "  [+] ELF Program Loader");
    } else if (strcmp(cmd, "cpuinfo") == 0) {
        window_add_line(window, "CPU Information");
        window_add_line(window, "---------------");
        window_add_line(window, "Architecture: x86_64");
        window_add_line(window, "Mode: Long Mode (64-bit)");
        window_add_line(window, "Features: CPUID detection enabled");
        window_add_line(window, "Instruction Sets:");
        window_add_line(window, "  [+] FPU (Floating Point Unit)");
        window_add_line(window, "  [+] MMX");
        window_add_line(window, "  [+] SSE/SSE2");
        window_add_line(window, "  [+] PAE (Physical Address Extension)");
        window_add_line(window, "  [+] TSC (Time Stamp Counter)");
    } else if (strcmp(cmd, "gpu") == 0) {
        window_add_line(window, "GPU Information");
        window_add_line(window, "---------------");
        window_add_line(window, "Vendor: VESA");
        window_add_line(window, "Driver: VBE Framebuffer");
        window_add_line(window, "Acceleration: Software rendering");
        window_add_line(window, "Buffers: Double-buffered");
    } else if (strcmp(cmd, "shutdown") == 0) {
        window_add_line(window, "");
        window_add_line(window, "Shutting down AetherOS...");
        window_add_line(window, "Goodbye!");
        for (size_t i = 0; i < ARRAY_SIZE(windows); ++i) {
            if (windows[i].used && i != (size_t)active_window) {
                windows[i].used = false;
            }
        }
        __asm__ volatile("hlt");
    } else if (strcmp(cmd, "reboot") == 0) {
        window_add_line(window, "");
        window_add_line(window, "Rebooting AetherOS...");
        for (size_t i = 0; i < ARRAY_SIZE(windows); ++i) {
            windows[i].used = false;
        }
        window_count = 0;
        active_window = -1;
        wm_init();
    } else if (strcmp(cmd, "run") == 0) {
        fs_node_t *node = ramfs_resolve(terminal_cwd, arg);
        if (!node || node->is_dir) {
            window_add_line(window, "run: file not found");
        } else {
            const char *ext = 0;
            for (size_t i = 0; node->name[i] != '\0'; ++i) {
                if (node->name[i] == '.') {
                    ext = &node->name[i];
                }
            }
            if (ext && strcmp(ext, ".lx") == 0) {
                create_app_window(node->name, (const char *)node->data);
            } else {
                window_add_line(window, "run: only .lx apps are supported in WM");
            }
        }
    } else {
        window_add_line(window, "unknown command");
    }
}

static void create_terminal_window(void) {
    int index = wm_create_window(0, 0, 0, 0, "Terminal");
    if (index < 0) {
        return;
    }
    windows[index].kind = WM_WINDOW_TERMINAL;
    window_add_line(&windows[index], "AetherOS graphics terminal");
    window_add_line(&windows[index], "Double-buffered framebuffer mode.");
    window_add_line(&windows[index], "Type help for commands.");
}

static void focus_next_window(void) {
    if (window_count == 0) {
        active_window = -1;
        return;
    }
    int start = active_window;
    for (size_t i = 0; i < ARRAY_SIZE(windows); ++i) {
        int index = (start + 1 + (int)i) % (int)ARRAY_SIZE(windows);
        if (windows[index].used) {
            wm_set_active(index);
            return;
        }
    }
}

void wm_remove_window(int index) {
    if (index < 0 || index >= (int)ARRAY_SIZE(windows) || !windows[index].used) {
        return;
    }
    windows[index].used = false;
    windows[index].active = false;
    if (window_count > 0) {
        window_count--;
    }
    if (active_window == index) {
        active_window = -1;
        focus_next_window();
    }
    wm_recalculate_layout();
}

void wm_handle_key(const key_event_t *event) {
    if (!event) {
        return;
    }

    if (event->alt) {
        if (event->type == KEY_ENTER) {
            create_terminal_window();
        } else if (event->type == KEY_TAB) {
            focus_next_window();
        } else if (event->type == KEY_CHAR && (event->ch == 'q' || event->ch == 'Q')) {
            wm_remove_window(active_window);
        }
        return;
    }

    wm_window_t *window = active_terminal();
    if (!window) {
        return;
    }

    if (event->type == KEY_CHAR) {
        if (window->input_len + 1 < sizeof(window->input)) {
            window->input[window->input_len++] = event->ch;
            window->input[window->input_len] = '\0';
            wm_request_redraw();
        }
    } else if (event->type == KEY_BACKSPACE) {
        if (window->input_len > 0) {
            window->input[--window->input_len] = '\0';
            wm_request_redraw();
        }
    } else if (event->type == KEY_ENTER) {
        terminal_run_command(window);
    }
}

void wm_init(void) {
    memset(windows, 0, sizeof(windows));
    window_count = 0;
    active_window = -1;
    terminal_cwd = ramfs_root();
    wm_dirty = true;
    wm_last_clock_second = 0;

    create_terminal_window();
    create_app_window("welcome.lx",
                      "Welcome to the graphics WM.\n"
                      "Window animation with flash effects.\n"
                      "Alt+Enter creates a window.\n"
                      "Alt+Tab switches focus (with animation).\n"
                      "Alt+Q closes the active window.\n"
                      "Try: sysinfo, cpuinfo, gpu, shutdown\n");
    wm_draw();
}

void wm_task(void *arg) {
    (void)arg;
    wm_init();
    for (;;) {
        key_event_t event;
        uint64_t now_tick = pit_ticks();
        uint64_t now_second = pit_ticks() / 100;
        if (now_second != wm_last_clock_second) {
            wm_last_clock_second = now_second;
            wm_request_redraw();
        }
        if (now_tick != wm_last_anim_tick) {
            wm_last_anim_tick = now_tick;
            wm_animate_layout();
        }
        if (keyboard_get_event(&event)) {
            wm_handle_key(&event);
            wm_draw();
        } else if (wm_dirty) {
            wm_draw();
        } else {
            __asm__ volatile ("hlt");
        }
    }
}
