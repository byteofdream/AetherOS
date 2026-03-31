#include "../include/app_api.h"
#include <string.h>

void app_entry(const app_syscalls_t *syscalls) {
    if (!syscalls || !syscalls->write) {
        return;
    }
    
    syscalls->write("\n");
    syscalls->write("╔═══════════════════════════════════╗\n");
    syscalls->write("║        AetherOS System Info        ║\n");
    syscalls->write("╚═══════════════════════════════════╝\n\n");
    
    syscalls->write("┌─ Host\n");
    syscalls->write("│  OS: AetherOS (x86_64)\n");
    syscalls->write("│  Arch: x86_64 / Long Mode\n");
    syscalls->write("├─ CPU\n");
    syscalls->write("│  Cores: 1 (Single-threaded scheduler)\n");
    syscalls->write("│  Mode: 64-bit\n");
    syscalls->write("├─ Memory\n");
    syscalls->write("│  Heap: Initialized\n");
    syscalls->write("├─ Storage\n");
    syscalls->write("│  Filesystem: RAMFS\n");
    syscalls->write("├─ Graphics\n");
    syscalls->write("│  Mode: VESA/VBE Framebuffer\n");
    syscalls->write("│  Buffer: Double-buffered\n");
    syscalls->write("├─ Devices\n");
    syscalls->write("│  2 PS/2 Controllers (Keyboard + Mouse)\n");
    syscalls->write("│  ATA Primary Channel (if available)\n");
    syscalls->write("│  PCI Bus Scanning\n");
    syscalls->write("├─ Kernel Features\n");
    syscalls->write("│  ✓ x86_64 Long Mode\n");
    syscalls->write("│  ✓ GDT / IDT / PIC / PIT\n");
    syscalls->write("│  ✓ Process Scheduler\n");
    syscalls->write("│  ✓ Memory Management\n");
    syscalls->write("│  ✓ Interrupt Handling\n");
    syscalls->write("│  ✓ Window Manager\n");
    syscalls->write("│  ✓ ELF Program Loader\n");
    syscalls->write("└─ Userland\n");
    syscalls->write("   Shell, File operations, Program execution\n\n");
}
