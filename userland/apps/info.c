#include "app_api.h"

void app_entry(const app_syscalls_t *syscalls) {
    syscalls->write("Minimal userspace application\n");
    syscalls->write("ABI: RDI -> syscall table\n");
}
