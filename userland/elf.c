#include "elf.h"
#include "app_api.h"
#include "console.h"
#include "string.h"

typedef struct {
    unsigned char e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} __attribute__((packed)) elf64_hdr_t;

typedef struct {
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
} __attribute__((packed)) elf64_phdr_t;

typedef void (*app_entry_t)(const app_syscalls_t *syscalls);

static void app_write(const char *text) {
    console_write(text);
}

static void app_write_hex(uint64_t value) {
    console_write_hex(value);
}

static void app_write_dec(uint64_t value) {
    console_write_dec(value);
}

bool elf_run(const void *image, size_t size) {
    if (size < sizeof(elf64_hdr_t)) {
        return false;
    }

    const elf64_hdr_t *hdr = image;
    if (memcmp(hdr->e_ident, "\x7F""ELF", 4) != 0 || hdr->e_ident[4] != 2 || hdr->e_machine != 62) {
        return false;
    }

    for (uint16_t i = 0; i < hdr->e_phnum; ++i) {
        const elf64_phdr_t *ph = (const elf64_phdr_t *)((const uint8_t *)image + hdr->e_phoff + i * hdr->e_phentsize);
        if (ph->p_type != 1) {
            continue;
        }
        if (ph->p_offset + ph->p_filesz > size) {
            return false;
        }
        memcpy((void *)(uintptr_t)ph->p_vaddr, (const uint8_t *)image + ph->p_offset, ph->p_filesz);
        if (ph->p_memsz > ph->p_filesz) {
            memset((void *)(uintptr_t)(ph->p_vaddr + ph->p_filesz), 0, ph->p_memsz - ph->p_filesz);
        }
    }

    static const app_syscalls_t syscalls = {
        .write = app_write,
        .write_hex = app_write_hex,
        .write_dec = app_write_dec
    };
    ((app_entry_t)(uintptr_t)hdr->e_entry)(&syscalls);
    return true;
}
