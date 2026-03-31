#ifndef KERNEL_MEMORY_H
#define KERNEL_MEMORY_H

#include "multiboot.h"

void memory_init(const boot_info_t *boot_info);
void *page_alloc(void);
void page_free(void *page);
uint64_t memory_total_bytes(void);

#endif
