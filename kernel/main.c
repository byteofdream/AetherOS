#include "common.h"
#include "console.h"
#include "graphics.h"
#include "multiboot.h"
#include "memory.h"
#include "heap.h"
#include "interrupts.h"
#include "pic.h"
#include "pit.h"
#include "scheduler.h"
#include "pci.h"
#include "usb.h"
#include "ps2.h"
#include "ata.h"
#include "ramfs.h"
#include "wm.h"
#include "cpu.h"

static boot_info_t boot_info;

static void graphics_boot_card(void) {
    if (!graphics_available()) {
        return;
    }

    graphics_clear(0x0B1220);
    draw_rect_vertical_gradient(0, 0, (int)graphics_width(), (int)graphics_height(), 0x101A2E, 0x0B1220);
    draw_rect(160, 130, 704, 260, 0x111A2E);
    draw_border(160, 130, 704, 260, 0x63C7FF);
    draw_rect_vertical_gradient(160, 130, 704, 56, 0x31598C, 0x20395A);
    draw_text_scaled(194, 144, "AetherOS", 0xFFFFFF, 3);
    draw_text_scaled(194, 214, "Initializing framebuffer compositor...", 0xD7E3FF, 2);
    draw_text_scaled(194, 244, "Theme engine, large bitmap font, and double buffering online.", 0xD7E3FF, 2);
    draw_text_scaled(194, 300, "Launching refined window manager...", 0xF6C85F, 2);
    graphics_present();
}

static void boot_draw_screen(void) {
    console_set_color(CONSOLE_COLOR_WHITE, CONSOLE_COLOR_BLUE);
    console_clear();
    console_set_color(CONSOLE_COLOR_YELLOW, CONSOLE_COLOR_BLUE);
    console_draw_box(1, 3, 11, 74);
    console_write_at(3, 8, "      _      _   _                 ___   ____");
    console_write_at(4, 8, "     / \\    / | | |_   ___ _ __   / _ \\ / ___|");
    console_write_at(5, 8, "    / _ \\   | | | __| / _ \\ '__| | | | | |    ");
    console_write_at(6, 8, "   / ___ \\  | | | |_ |  __/ |    | |_| | |___ ");
    console_write_at(7, 8, "  /_/   \\_\\ |_|  \\__| \\___|_|     \\___/ \\____|");
    console_set_color(CONSOLE_COLOR_LIGHT_CYAN, CONSOLE_COLOR_BLUE);
    console_write_at(9, 8, "A cleaner little OS with a real kernel, shell and drivers.");
    console_set_color(CONSOLE_COLOR_WHITE, CONSOLE_COLOR_BLUE);
    console_write_at(13, 5, "[BOOT]");
}

static void boot_status(size_t line, const char *label, const char *value, uint8_t value_color) {
    console_set_color(CONSOLE_COLOR_LIGHT_GREY, CONSOLE_COLOR_BLUE);
    console_write_at(line, 12, "                              ");
    console_write_at(line, 12, label);
    console_write_at(line, 36, ": ");
    console_set_color(value_color, CONSOLE_COLOR_BLUE);
    console_write_at(line, 38, value);
}

static void idle_task(void *arg) {
    (void)arg;
    for (;;) {
        __asm__ volatile ("hlt");
    }
}

void kernel_entry64(uint32_t mbi_addr) {
    __asm__ volatile ("cli");

    console_init();
    boot_draw_screen();
    boot_status(15, "loader", "GRUB / Multiboot2", CONSOLE_COLOR_LIGHT_GREEN);

    multiboot_parse(mbi_addr, &boot_info);
    boot_status(16, "cpu mode", "x86_64 long mode", CONSOLE_COLOR_LIGHT_GREEN);
    memory_init(&boot_info);
    heap_init();
    cpu_init();
    graphics_init(&boot_info);
    graphics_boot_card();
    console_set_color(CONSOLE_COLOR_YELLOW, CONSOLE_COLOR_BLUE);
    console_move_to(17, 38);
    console_write_dec(memory_total_bytes() / (1024 * 1024));
    console_write(" MB detected");

    pci_init();
    pic_remap(32, 40);
    interrupts_init();
    pit_init(100);
    boot_status(18, "interrupts", "IDT / PIC / PIT online", CONSOLE_COLOR_LIGHT_GREEN);

    scheduler_init();
    ramfs_init();
    boot_status(19, "filesystem", "RAMFS mounted", CONSOLE_COLOR_LIGHT_GREEN);

    if (ata_init()) {
        boot_status(20, "storage", "ATA primary channel ready", CONSOLE_COLOR_LIGHT_GREEN);
    } else {
        boot_status(20, "storage", "ATA not detected", CONSOLE_COLOR_YELLOW);
    }

    ps2_init();
    usb_init();
    boot_status(21, "input", "PS/2 keyboard + mouse", CONSOLE_COLOR_LIGHT_GREEN);

    scheduler_create_task("wm", wm_task, 0);
    scheduler_create_task("idle", idle_task, 0);
    boot_status(22, "services", "window manager armed", CONSOLE_COLOR_LIGHT_GREEN);

    __asm__ volatile ("sti");
    console_set_color(CONSOLE_COLOR_WHITE, CONSOLE_COLOR_BLUE);
    console_write_at(23, 5, "Launching window manager...");
    scheduler_start();

    console_writeln("Scheduler returned");
    for (;;) {
        __asm__ volatile ("hlt");
    }
}
