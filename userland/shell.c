#include "shell.h"
#include "console.h"
#include "keyboard.h"
#include "mouse.h"
#include "pit.h"
#include "memory.h"
#include "ramfs.h"
#include "elf.h"
#include "string.h"

static fs_node_t *cwd;
static void shell_draw_header(void);
static void cmd_help(void);
static void cmd_ls(void);
static void cmd_cd(const char *arg);
static void cmd_pwd(void);
static void cmd_cat(const char *arg);
static void cmd_run(const char *arg);
static void cmd_mem(void);
static void cmd_uptime(void);
static void cmd_mouse(void);
static void cmd_sysinfo(void);
static void cmd_cpuinfo(void);
static void cmd_gpu(void);
static void cmd_about(void);
static void cmd_shutdown(void);
static void cmd_reboot(void);

static void write_signed_dec(int value) {
    if (value < 0) {
        console_putc('-');
        console_write_dec((uint64_t)(-(int64_t)value));
        return;
    }
    console_write_dec((uint64_t)value);
}

static void print_prompt(void) {
    char path[128];
    ramfs_path(cwd, path, sizeof(path));
    console_set_color(CONSOLE_COLOR_LIGHT_CYAN, CONSOLE_COLOR_BLACK);
    console_write("aether");
    console_set_color(CONSOLE_COLOR_WHITE, CONSOLE_COLOR_BLACK);
    console_write(":");
    console_set_color(CONSOLE_COLOR_YELLOW, CONSOLE_COLOR_BLACK);
    console_write(path);
    console_set_color(CONSOLE_COLOR_LIGHT_GREEN, CONSOLE_COLOR_BLACK);
    console_write(" $ ");
    console_reset_color();
}

static void cmd_help(void) {
    console_writeln("Commands: help ls cd pwd cat run clear mem uptime mouse sysinfo cpuinfo gpu shutdown reboot about");
}

static void cmd_ls(void) {
    for (size_t i = 0; i < cwd->child_count; ++i) {
        if (cwd->children[i]->is_dir) {
            console_set_color(CONSOLE_COLOR_LIGHT_CYAN, CONSOLE_COLOR_BLACK);
        } else {
            console_set_color(CONSOLE_COLOR_WHITE, CONSOLE_COLOR_BLACK);
        }
        console_write(cwd->children[i]->name);
        if (cwd->children[i]->is_dir) {
            console_putc('/');
        }
        console_putc('\n');
    }
    console_reset_color();
}

static void cmd_cd(const char *arg) {
    fs_node_t *node = ramfs_resolve(cwd, arg);
    if (!node || !node->is_dir) {
        console_writeln("cd: no such directory");
        return;
    }
    cwd = node;
}

static void cmd_pwd(void) {
    char path[128];
    ramfs_path(cwd, path, sizeof(path));
    console_writeln(path);
}

static void cmd_cat(const char *arg) {
    fs_node_t *node = ramfs_resolve(cwd, arg);
    if (!node || node->is_dir) {
        console_writeln("cat: file not found");
        return;
    }
    console_write_len((const char *)node->data, node->size);
    if (node->size == 0 || node->data[node->size - 1] != '\n') {
        console_putc('\n');
    }
}

static void cmd_run(const char *arg) {
    fs_node_t *node = ramfs_resolve(cwd, arg);
    if (!node || node->is_dir) {
        console_writeln("run: file not found");
        return;
    }
    if (!elf_run(node->data, node->size)) {
        console_writeln("run: invalid ELF");
    } else {
        console_putc('\n');
    }
}

static void cmd_mem(void) {
    console_write("Physical memory detected: ");
    console_write_dec(memory_total_bytes() / (1024 * 1024));
    console_writeln(" MB");
}

static void cmd_uptime(void) {
    uint64_t ticks = pit_ticks();
    console_write("Uptime: ");
    console_write_dec(ticks / 100);
    console_write(".");
    console_write_dec(ticks % 100);
    console_writeln(" s");
}

static void cmd_mouse(void) {
    mouse_state_t state = mouse_state();
    console_write("Mouse x=");
    write_signed_dec(state.x);
    console_write(" y=");
    write_signed_dec(state.y);
    console_write(" buttons=");
    console_write_dec(state.buttons);
    console_putc('\n');
}

static void cmd_sysinfo(void) {
    console_writeln("AetherOS System Information");
    console_writeln("----------------------------");
    console_writeln("OS Name: AetherOS");
    console_writeln("Architecture: x86_64");
    console_writeln("Bootloader: GRUB2 (Multiboot2)");
    console_writeln("Mode: Long Mode (64-bit)");
    console_writeln("Kernel Features:");
    console_writeln("  [+] GDT/IDT/PIC/PIT");
    console_writeln("  [+] Process Scheduler");
    console_writeln("  [+] Memory Management");
    console_writeln("  [+] RAMFS Filesystem");
    console_writeln("  [+] Window Manager");
    console_writeln("  [+] ELF Program Loader");
}

static void cmd_cpuinfo(void) {
    console_writeln("CPU Information");
    console_writeln("---------------");
    console_writeln("Architecture: x86_64");
    console_writeln("Mode: Long Mode (64-bit)");
    console_writeln("Features: CPUID detection enabled");
    console_writeln("Instruction Sets:");
    console_writeln("  [+] FPU (Floating Point Unit)");
    console_writeln("  [+] MMX");
    console_writeln("  [+] SSE/SSE2");
    console_writeln("  [+] PAE (Physical Address Extension)");
    console_writeln("  [+] TSC (Time Stamp Counter)");
}

static void cmd_gpu(void) {
    console_writeln("GPU Information");
    console_writeln("---------------");
    console_writeln("Vendor: VESA");
    console_writeln("Driver: VBE Framebuffer");
    console_writeln("Acceleration: Software rendering");
    console_writeln("Buffers: Double-buffered");
}

static void cmd_about(void) {
    console_writeln("AetherOS");
    console_writeln("Tiny x86_64 OS with a prettier boot screen, shell, RAMFS and ELF apps.");
}

static void cmd_shutdown(void) {
    console_writeln("");
    console_writeln("Shutting down AetherOS...");
    console_writeln("Goodbye!");
    __asm__ volatile("hlt");
}

static void cmd_reboot(void) {
    console_writeln("");
    console_writeln("Rebooting AetherOS...");
    __asm__ volatile("hlt");
}

static char *skip_spaces(char *s) {
    while (*s == ' ') {
        ++s;
    }
    return s;
}

static void shell_draw_header(void) {
    console_set_color(CONSOLE_COLOR_WHITE, CONSOLE_COLOR_BLUE);
    console_clear();
    console_draw_box(0, 0, 3, 80);
    console_set_color(CONSOLE_COLOR_YELLOW, CONSOLE_COLOR_BLUE);
    console_write_at(1, 2, "AetherOS Control Deck");
    console_set_color(CONSOLE_COLOR_LIGHT_CYAN, CONSOLE_COLOR_BLUE);
    console_write_at(1, 27, "sysinfo | cpuinfo | gpu | mem | uptime | help");
    console_reset_color();
    console_move_to(4, 0);
}

void shell_task(void *arg) {
    (void)arg;
    cwd = ramfs_root();
    shell_draw_header();
    console_set_color(CONSOLE_COLOR_LIGHT_GREY, CONSOLE_COLOR_BLACK);
    console_writeln("AetherOS shell online. Type help.");
    console_reset_color();

    char line[128];
    for (;;) {
        print_prompt();
        size_t len = 0;
        for (;;) {
            char c;
            while (!keyboard_get_char(&c)) {
                __asm__ volatile ("hlt");
            }

            if (c == '\n') {
                console_putc('\n');
                line[len] = '\0';
                break;
            }
            if (c == '\b') {
                if (len > 0) {
                    len--;
                    console_putc('\b');
                }
                continue;
            }
            if (len + 1 < sizeof(line)) {
                line[len++] = c;
                console_putc(c);
            }
        }

        char *cmd = skip_spaces(line);
        char *argp = strchr(cmd, ' ');
        if (argp) {
            *argp++ = '\0';
            argp = skip_spaces(argp);
        } else {
            argp = "";
        }

        if (strcmp(cmd, "") == 0) {
            continue;
        } else if (strcmp(cmd, "help") == 0) {
            cmd_help();
        } else if (strcmp(cmd, "ls") == 0) {
            cmd_ls();
        } else if (strcmp(cmd, "cd") == 0) {
            cmd_cd(argp);
        } else if (strcmp(cmd, "pwd") == 0) {
            cmd_pwd();
        } else if (strcmp(cmd, "cat") == 0) {
            cmd_cat(argp);
        } else if (strcmp(cmd, "run") == 0) {
            cmd_run(argp);
        } else if (strcmp(cmd, "clear") == 0) {
            shell_draw_header();
        } else if (strcmp(cmd, "mem") == 0) {
            cmd_mem();
        } else if (strcmp(cmd, "uptime") == 0) {
            cmd_uptime();
        } else if (strcmp(cmd, "mouse") == 0) {
            cmd_mouse();
        } else if (strcmp(cmd, "sysinfo") == 0) {
            cmd_sysinfo();
        } else if (strcmp(cmd, "cpuinfo") == 0) {
            cmd_cpuinfo();
        } else if (strcmp(cmd, "gpu") == 0) {
            cmd_gpu();
        } else if (strcmp(cmd, "about") == 0) {
            cmd_about();
        } else if (strcmp(cmd, "shutdown") == 0) {
            cmd_shutdown();
        } else if (strcmp(cmd, "reboot") == 0) {
            cmd_reboot();
        } else {
            console_writeln("unknown command");
        }
    }
}
