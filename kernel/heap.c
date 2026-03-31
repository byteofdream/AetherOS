#include "heap.h"
#include "string.h"

static uint8_t heap_area[8 * 1024 * 1024];
static size_t heap_offset;

void heap_init(void) {
    heap_offset = 0;
}

void *kmalloc(size_t size) {
    if (size == 0) {
        return 0;
    }
    size = (size + 15) & ~((size_t)15);
    if (heap_offset + size > sizeof(heap_area)) {
        return 0;
    }
    void *ptr = &heap_area[heap_offset];
    heap_offset += size;
    return ptr;
}

void *kcalloc(size_t count, size_t size) {
    size_t total = count * size;
    void *ptr = kmalloc(total);
    if (ptr) {
        memset(ptr, 0, total);
    }
    return ptr;
}
