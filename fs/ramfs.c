#include "ramfs.h"
#include "heap.h"
#include "string.h"

extern const uint8_t _binary_build_userland_hello_elf_start[];
extern const uint8_t _binary_build_userland_hello_elf_end[];
extern const uint8_t _binary_build_userland_info_elf_start[];
extern const uint8_t _binary_build_userland_info_elf_end[];

static fs_node_t *root_node;

static fs_node_t *alloc_node(const char *name, bool is_dir) {
    fs_node_t *node = kcalloc(1, sizeof(fs_node_t));
    if (!node) {
        return 0;
    }
    strncpy(node->name, name, sizeof(node->name) - 1);
    node->is_dir = is_dir;
    return node;
}

void ramfs_init(void) {
    static const char readme_text[] =
        "AetherOS RAM filesystem\n"
        "Type help to see available commands.\n"
        "This build focuses on aesthetics and a friendlier shell.\n";
    static const char about_text[] =
        "AetherOS is a tiny x86_64 hobby operating system with a cooperative scheduler,\n"
        "PS/2 input, ATA PIO storage routines, a RAM filesystem and an ELF loader.\n";
    static const char art_text[] =
        "AetherOS\n"
        "Booting style matters too.\n";
    static const char notes_app[] =
        "notes.lx\n"
        "This is a simple text application window.\n"
        "It is rendered by the WM without mouse support.\n";
    static const char sysinfo_app[] =
        "sysinfo.lx\n"
        "Window manager demo app\n"
        "Keyboard only\n"
        "Stable VGA text rendering\n";

    root_node = alloc_node("/", true);
    fs_node_t *bin = ramfs_mkdir(root_node, "bin");
    fs_node_t *docs = ramfs_mkdir(root_node, "docs");
    fs_node_t *art = ramfs_mkdir(root_node, "art");
    fs_node_t *apps = ramfs_mkdir(root_node, "apps");

    ramfs_create(bin, "hello.elf",
                 _binary_build_userland_hello_elf_start,
                 (size_t)(_binary_build_userland_hello_elf_end - _binary_build_userland_hello_elf_start));
    ramfs_create(bin, "info.elf",
                 _binary_build_userland_info_elf_start,
                 (size_t)(_binary_build_userland_info_elf_end - _binary_build_userland_info_elf_start));
    ramfs_create(docs, "readme.txt", readme_text, sizeof(readme_text) - 1);
    ramfs_create(docs, "about.txt", about_text, sizeof(about_text) - 1);
    ramfs_create(art, "boot.txt", art_text, sizeof(art_text) - 1);
    ramfs_create(apps, "notes.lx", notes_app, sizeof(notes_app) - 1);
    ramfs_create(apps, "sysinfo.lx", sysinfo_app, sizeof(sysinfo_app) - 1);
}

fs_node_t *ramfs_root(void) {
    return root_node;
}

fs_node_t *ramfs_mkdir(fs_node_t *parent, const char *name) {
    fs_node_t *node = alloc_node(name, true);
    if (!node || !parent || parent->child_count >= ARRAY_SIZE(parent->children)) {
        return 0;
    }
    node->parent = parent;
    parent->children[parent->child_count++] = node;
    return node;
}

fs_node_t *ramfs_create(fs_node_t *parent, const char *name, const void *data, size_t size) {
    fs_node_t *node = alloc_node(name, false);
    if (!node || !parent || parent->child_count >= ARRAY_SIZE(parent->children)) {
        return 0;
    }
    node->parent = parent;
    node->capacity = size ? size : 64;
    node->data = kmalloc(node->capacity);
    if (size && data) {
        memcpy(node->data, data, size);
    }
    node->size = size;
    parent->children[parent->child_count++] = node;
    return node;
}

bool ramfs_write(fs_node_t *node, const void *data, size_t size) {
    if (!node || node->is_dir) {
        return false;
    }
    if (size > node->capacity) {
        uint8_t *new_data = kmalloc(size);
        if (!new_data) {
            return false;
        }
        node->data = new_data;
        node->capacity = size;
    }
    memcpy(node->data, data, size);
    node->size = size;
    return true;
}

static fs_node_t *child_lookup(fs_node_t *dir, const char *name) {
    if (!dir || !dir->is_dir) {
        return 0;
    }
    for (size_t i = 0; i < dir->child_count; ++i) {
        if (strcmp(dir->children[i]->name, name) == 0) {
            return dir->children[i];
        }
    }
    return 0;
}

fs_node_t *ramfs_resolve(fs_node_t *cwd, const char *path) {
    fs_node_t *node = (path[0] == '/') ? root_node : cwd;
    char token[32];
    size_t pos = 0;

    if (!node) {
        return 0;
    }

    for (size_t i = 0;; ++i) {
        char c = path[i];
        if (c == '/' || c == '\0') {
            token[pos] = '\0';
            if (pos > 0) {
                if (strcmp(token, ".") == 0) {
                } else if (strcmp(token, "..") == 0) {
                    if (node->parent) {
                        node = node->parent;
                    }
                } else {
                    node = child_lookup(node, token);
                    if (!node) {
                        return 0;
                    }
                }
            }
            pos = 0;
            if (c == '\0') {
                return node;
            }
        } else if (pos + 1 < sizeof(token)) {
            token[pos++] = c;
        }
    }
}

void ramfs_path(fs_node_t *node, char *buffer, size_t size) {
    fs_node_t *parts[16];
    size_t count = 0;
    size_t pos = 0;

    if (!buffer || size == 0) {
        return;
    }

    if (!node || node == root_node) {
        if (size > 1) {
            buffer[0] = '/';
            buffer[1] = '\0';
        } else {
            buffer[0] = '\0';
        }
        return;
    }

    while (node && node != root_node && count < ARRAY_SIZE(parts)) {
        parts[count++] = node;
        node = node->parent;
    }

    buffer[pos++] = '/';
    for (size_t i = count; i > 0; --i) {
        const char *name = parts[i - 1]->name;
        for (size_t j = 0; name[j] != '\0' && pos + 1 < size; ++j) {
            buffer[pos++] = name[j];
        }
        if (i > 1 && pos + 1 < size) {
            buffer[pos++] = '/';
        }
    }
    buffer[pos] = '\0';
}
