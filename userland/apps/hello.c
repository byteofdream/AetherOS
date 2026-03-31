#include "app_api.h"

void app_entry(const app_syscalls_t *syscalls) {
    syscalls->write("Hello from ELF app\n");
}
