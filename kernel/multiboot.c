#include "multiboot.h"
#include "string.h"

typedef struct {
    uint32_t total_size;
    uint32_t reserved;
} mb2_info_t;

typedef struct {
    uint32_t type;
    uint32_t size;
} mb2_tag_t;

typedef struct {
    uint32_t type;
    uint32_t size;
    uint64_t addr;
    uint32_t pitch;
    uint32_t width;
    uint32_t height;
    uint8_t bpp;
    uint8_t framebuffer_type;
    uint16_t reserved;
} mb2_tag_fb_t;

typedef struct {
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
} mb2_tag_mmap_t;

typedef struct {
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
    uint32_t reserved;
} __attribute__((packed)) mb2_mmap_entry_t;

void multiboot_parse(uint32_t mbi_addr, boot_info_t *info) {
    memset(info, 0, sizeof(*info));
    const mb2_info_t *mbi = (const mb2_info_t *)(uintptr_t)mbi_addr;
    const uint8_t *cursor = (const uint8_t *)mbi + 8;
    const uint8_t *end = (const uint8_t *)mbi + mbi->total_size;

    while (cursor < end) {
        const mb2_tag_t *tag = (const mb2_tag_t *)cursor;
        if (tag->type == 0) {
            break;
        }
        if (tag->size < 8) {
            break;
        }

        if (tag->type == 8) {
            const mb2_tag_fb_t *fb = (const mb2_tag_fb_t *)tag;
            info->framebuffer_addr = fb->addr;
            info->framebuffer_pitch = fb->pitch;
            info->framebuffer_width = fb->width;
            info->framebuffer_height = fb->height;
            info->framebuffer_bpp = fb->bpp;
        } else if (tag->type == 6) {
            const mb2_tag_mmap_t *mmap = (const mb2_tag_mmap_t *)tag;
            if (mmap->entry_size == 0) {
                break;
            }
            const uint8_t *entry_ptr = cursor + sizeof(*mmap);
            const uint8_t *entry_end = cursor + tag->size;
            while (entry_ptr < entry_end && info->region_count < ARRAY_SIZE(info->regions)) {
                const mb2_mmap_entry_t *entry = (const mb2_mmap_entry_t *)entry_ptr;
                info->regions[info->region_count].base = entry->base_addr;
                info->regions[info->region_count].length = entry->length;
                info->regions[info->region_count].type = entry->type;
                info->region_count++;
                entry_ptr += mmap->entry_size;
            }
        }

        cursor += (tag->size + 7) & ~7u;
    }
}
