#ifndef KERNEL_MULTIBOOT_H
#define KERNEL_MULTIBOOT_H

#include "common.h"

typedef struct {
    uint64_t base;
    uint64_t length;
    uint32_t type;
} memory_region_t;

typedef struct {
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bpp;
    memory_region_t regions[64];
    size_t region_count;
} boot_info_t;

void multiboot_parse(uint32_t mbi_addr, boot_info_t *info);

#endif
