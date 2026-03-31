#ifndef USERLAND_APP_API_H
#define USERLAND_APP_API_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    void (*write)(const char *text);
    void (*write_hex)(uint64_t value);
    void (*write_dec)(uint64_t value);
} app_syscalls_t;

#endif
