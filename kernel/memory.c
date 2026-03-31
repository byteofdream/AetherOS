#include "memory.h"
#include "string.h"

static uint8_t *free_pages[8192];
static size_t free_page_count;
static uint64_t total_bytes;

void memory_init(const boot_info_t *boot_info) {
    free_page_count = 0;
    total_bytes = 0;

    if (boot_info->region_count == 0) {
        total_bytes = 64ULL * 1024 * 1024;
        for (uint64_t addr = 0x400000; addr < 0x4000000 && free_page_count < ARRAY_SIZE(free_pages); addr += 0x1000) {
            free_pages[free_page_count++] = (uint8_t *)(uintptr_t)addr;
        }
        return;
    }

    for (size_t i = 0; i < boot_info->region_count; ++i) {
        const memory_region_t *region = &boot_info->regions[i];
        if (region->type != 1) {
            continue;
        }
        total_bytes += region->length;
        uint64_t start = region->base;
        uint64_t end = region->base + region->length;
        if (end <= 0x200000) {
            continue;
        }
        if (start < 0x200000) {
            start = 0x200000;
        }
        start = (start + 0xFFF) & ~0xFFFULL;
        for (uint64_t addr = start; addr + 0x1000 <= end && free_page_count < ARRAY_SIZE(free_pages); addr += 0x1000) {
            free_pages[free_page_count++] = (uint8_t *)(uintptr_t)addr;
        }
    }
}

void *page_alloc(void) {
    if (free_page_count == 0) {
        return 0;
    }
    return free_pages[--free_page_count];
}

void page_free(void *page) {
    if (page && free_page_count < ARRAY_SIZE(free_pages)) {
        free_pages[free_page_count++] = page;
    }
}

uint64_t memory_total_bytes(void) {
    return total_bytes;
}
