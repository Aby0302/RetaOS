#include <kernel/vfs.h>
#include <kernel/kalloc.h>
#include <kernel/console.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include "fs/fat32.h"
#include "fs/fat32_vfs.h"

// Global variables from fat32.c
extern fat32_boot_sector_t g_boot_sector;
extern int g_initialized;

// Forward declarations
static ssize_t fat32_read(vfs_node_t* node, uint32_t offset, void* buffer, size_t size);
static ssize_t fat32_write(vfs_node_t* node, uint32_t offset, const void* buffer, size_t size);
static int fat32_open(vfs_node_t* node, uint32_t flags);
static int fat32_close(vfs_node_t* node);
static vfs_dirent_t fat32_vfs_readdir(vfs_node_t* node, uint32_t index);
static int fat32_finddir(vfs_node_t* node, const char* name, vfs_node_t** out_node);

// Private data structure for FAT32 file handles
typedef struct {
    uint32_t cluster;
    uint32_t size;
    uint32_t pos;
    uint8_t is_dir;
} fat32_file_private_t;

// Create a new VFS node for a FAT32 file/directory
static vfs_node_t* fat32_create_node(const char* name, uint32_t size, int is_dir, uint32_t cluster) {
    vfs_node_t node = {0};


    strncpy(node.name, name, VFS_NAME_MAX - 1);
    node.name[VFS_NAME_MAX - 1] = '\0';
    node.size = size;
    node.flags = is_dir ? VFS_DIRECTORY : VFS_FILE;

    node.read = fat32_read;
    node.write = fat32_write;
    node.open = fat32_open;
    node.close = fat32_close;
    node.readdir = is_dir ? fat32_vfs_readdir : NULL;
    node.finddir = is_dir ? fat32_finddir : NULL;
    // Allocate private data
    fat32_file_private_t* priv = (fat32_file_private_t*)kmalloc(sizeof(fat32_file_private_t));
    if (!priv) {
        console_puts("FAT32: Failed to allocate private data\n");
        return NULL; // Return NULL on error
    }

    priv->cluster = cluster;
    priv->size = size;
    priv->pos = 0;
    priv->is_dir = is_dir;

    node.priv = priv;

    // Allocate memory for the node
    vfs_node_t* node_ptr = (vfs_node_t*)kmalloc(sizeof(vfs_node_t));
    if (!node_ptr) {
        kfree(priv);
        console_puts("FAT32: Failed to allocate node memory\n");
        return NULL;
    }
    *node_ptr = node;

    console_puts("FAT32: Created node\n");

    return node_ptr;
}

// Read from a file
static ssize_t fat32_read(vfs_node_t* node, uint32_t offset, void* buffer, size_t size) {
    if (!node || !node->priv || !buffer) return -EINVAL;

    fat32_file_private_t* priv = (fat32_file_private_t*)node->priv;
    if (priv->is_dir) return -EISDIR;
    if (offset >= priv->size) return 0;

    if (offset + size > priv->size) {
        size = priv->size - offset;
    }

    // Call the FAT32 read function with the correct signature
    ssize_t bytes_read = fat32_read_file_data(priv->cluster, offset, buffer, size);
    if (bytes_read < 0) {
        console_puts("FAT32: Read error\n");
        return bytes_read;
    }

    priv->pos = offset + bytes_read;
    return bytes_read;
}

// Write to a file (not implemented)
static ssize_t fat32_write(vfs_node_t* node, uint32_t offset, const void* buffer, size_t size) {
    (void)node; (void)offset; (void)buffer; (void)size;
    return -ENOSYS;
}

// Open a file/directory
static int fat32_open(vfs_node_t* node, uint32_t flags) {
    (void)flags; // Unused parameter
    if (!node || !node->priv) return -EINVAL;

    fat32_file_private_t* priv = (fat32_file_private_t*)node->priv;
    priv->pos = 0;

    console_puts("FAT32: Opened file\n");

    return 0;
}

// Close a file/directory
static int fat32_close(vfs_node_t* node) {
    if (!node || !node->priv) return -EINVAL;

    fat32_file_private_t* priv = (fat32_file_private_t*)node->priv;

    console_puts("FAT32: Closed file\n");

    return 0;
}

// Read a directory entry
static vfs_dirent_t fat32_vfs_readdir(vfs_node_t* node, uint32_t index) {
    if (!node || !node->priv) {
        vfs_dirent_t empty = {0};
        return empty;
    }

    fat32_file_private_t* priv = (fat32_file_private_t*)node->priv;
    if (!priv->is_dir) {
        vfs_dirent_t empty = {0};
        return empty;
    }

    static vfs_dirent_t dirent;
    fat32_dir_t dir_entry;

    // Special case: . entry
    if (index == 0) {
        strcpy(dirent.name, ".");
        dirent.size = 0;
        dirent.is_dir = 1;
        return dirent;
    }

    // Special case: .. entry
    if (index == 1) {
        strcpy(dirent.name, "..");
        dirent.size = 0;
        dirent.is_dir = 1;
        return dirent;
    }

    // Regular entries (index - 2 because of . and ..)
    if (fat32_readdir_index(priv->cluster, index - 2, &dir_entry) != 0) {
        vfs_dirent_t empty = {0};
        return empty;
    }

    strncpy(dirent.name, dir_entry.name, VFS_NAME_MAX - 1);
    dirent.name[VFS_NAME_MAX - 1] = '\0';
    dirent.size = dir_entry.size;
    dirent.is_dir = (dir_entry.attributes & ATTR_DIRECTORY) != 0;

    return dirent;
}

// Update the function signature to match finddir_type_t
// typedef int (*finddir_type_t)(vfs_node_t*, const char*, vfs_node_t**);

// Update fat32_finddir implementation
static int fat32_finddir(vfs_node_t* node, const char* name, vfs_node_t** out_node) {
    if (!node || !node->priv || !name || !out_node) return -EINVAL;

    fat32_file_private_t* priv = (fat32_file_private_t*)node->priv;
    if (!priv->is_dir) return -ENOTDIR;

    // Handle special entries
    if (strcmp(name, ".") == 0) {
        *out_node = node; // Updated to dereference out_node
        return 0;
    }
    if (strcmp(name, "..") == 0) {
        vfs_node_t* root = vfs_get_root();
        if (root) {
            *out_node = root; // Updated to dereference out_node
            return 0;
        }
        return -ENOENT;
    }

    // Search for the entry
    fat32_dir_t dir_entry;
    if (fat32_find_entry(priv->cluster, name, &dir_entry) != 0) {
        return -ENOENT;
    }

    bool is_dir = (dir_entry.attributes & ATTR_DIRECTORY) != 0;
    *out_node = fat32_create_node(name, dir_entry.size, is_dir, dir_entry.cluster); // Updated field
    return 0;
}

// Mount a FAT32 filesystem
vfs_node_t* fat32_mount(const char* device) {
    (void)device; // Unused parameter
    if (fat32_init() != 0) {
        console_puts("FAT32: Failed to initialize filesystem\n");
        return NULL;
    }

    vfs_node_t* root = fat32_create_node("", 0, 1, g_boot_sector.root_cluster);
    if (!root) {
        console_puts("FAT32: Failed to create root node\n");
        return NULL;
    }

    strncpy(root->name, "/", VFS_NAME_MAX);
    console_puts("FAT32: Mounted filesystem\n");

    return root;
}

// Unmount a FAT32 filesystem
int fat32_umount(vfs_node_t* node) {
    if (!node || !node->priv) return -EINVAL;

    fat32_file_private_t* priv = (fat32_file_private_t*)node->priv;

    // Free private data
    if (node->priv) {
        kfree(node->priv);
        node->priv = NULL;
    }

    // Note: The node itself is passed by reference, so we can't free it here
    // The caller is responsible for managing the node's memory

    return 0;
}
