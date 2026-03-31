#ifndef KERNEL_HEAP_H
#define KERNEL_HEAP_H

#include "common.h"

void heap_init(void);
void *kmalloc(size_t size);
void *kcalloc(size_t count, size_t size);

#endif
