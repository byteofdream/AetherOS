#ifndef FS_RAMFS_H
#define FS_RAMFS_H

#include "common.h"

typedef struct fs_node {
    char name[32];
    bool is_dir;
    struct fs_node *parent;
    struct fs_node *children[16];
    size_t child_count;
    uint8_t *data;
    size_t size;
    size_t capacity;
} fs_node_t;

void ramfs_init(void);
fs_node_t *ramfs_root(void);
fs_node_t *ramfs_resolve(fs_node_t *cwd, const char *path);
fs_node_t *ramfs_mkdir(fs_node_t *parent, const char *name);
fs_node_t *ramfs_create(fs_node_t *parent, const char *name, const void *data, size_t size);
bool ramfs_write(fs_node_t *node, const void *data, size_t size);
void ramfs_path(fs_node_t *node, char *buffer, size_t size);

#endif
