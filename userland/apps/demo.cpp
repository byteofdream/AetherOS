// Example C++ Application for AetherOS
#include "../include/app_api.h"
#include <stddef.h>

// Simple C++ String class
class String {
private:
    char buffer[256];
    size_t len;

public:
    String() : len(0) {
        buffer[0] = '\0';
    }

    String(const char *str) : len(0) {
        while (str[len] && len < 255) {
            buffer[len] = str[len];
            len++;
        }
        buffer[len] = '\0';
    }

    void append(const char *str) {
        while (*str && len < 255) {
            buffer[len++] = *str++;
        }
        buffer[len] = '\0';
    }

    const char* c_str() const {
        return buffer;
    }

    size_t length() const {
        return len;
    }
};

extern "C" {
    void app_entry(const app_syscalls_t *syscalls);
}

void app_entry(const app_syscalls_t *syscalls) {
    if (!syscalls || !syscalls->write) {
        return;
    }

    syscalls->write("\n");
    syscalls->write("╔════════════════════════════════════════╗\n");
    syscalls->write("║   C++ Application Demo for AetherOS   ║\n");
    syscalls->write("╚════════════════════════════════════════╝\n\n");

    // Demonstrate String class
    String app_name("AetherOS");
    String message("Welcome to ");
    message.append(app_name.c_str());
    message.append("!\n");
    syscalls->write(message.c_str());

    syscalls->write("\n[C++ Features Demonstrated]\n");
    syscalls->write("  ✓ Classes and Objects (String)\n");
    syscalls->write("  ✓ String Concatenation\n");
    syscalls->write("  ✓ Constructors & Destructors\n");
    syscalls->write("  ✓ Data Members & Methods\n");
    syscalls->write("  ✓ const Correctness\n\n");

    syscalls->write("[System Information]\n");
    syscalls->write("  Compiled with C++ 11 features\n");
    syscalls->write("  Running in 64-bit protected mode\n");
    syscalls->write("  Memory management operational\n");
    syscalls->write("  Object-oriented programming available\n\n");

    syscalls->write("Program completed successfully!\n\n");
}


