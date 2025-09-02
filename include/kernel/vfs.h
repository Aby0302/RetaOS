#ifndef _VFS_H
#define _VFS_H

#include <stdint.h>
#include <stddef.h>
#include <sys/stat.h>  // struct stat and mode macros
#include <sys/types.h>
#include <errno.h>

// Remove local time_t and stat definitions to avoid conflicts

// Error codes
#ifndef ENOENT
#define ENOENT       2  // No such file or directory
#endif
#ifndef EIO
#define EIO          5  // I/O error
#endif
#ifndef EBADF
#define EBADF        9  // Bad file number
#endif
#ifndef ENOMEM
#define ENOMEM      12  // Out of memory
#endif
#ifndef EACCES
#define EACCES      13  // Permission denied
#endif
#ifndef EEXIST
#define EEXIST      17  // File exists
#endif
#ifndef ENOTDIR
#define ENOTDIR     20  // Not a directory
#endif
#ifndef EISDIR
#define EISDIR      21  // Is a directory
#endif
#ifndef EINVAL
#define EINVAL      22  // Invalid argument
#endif
#ifndef ENFILE
#define ENFILE      23  // Too many open files in system
#endif
#ifndef EMFILE
#define EMFILE      24  // Too many open files
#endif
#ifndef ENOSPC
#define ENOSPC      28  // No space left on device
#endif
#ifndef EROFS
#define EROFS       30  // Read-only file system
#endif
#ifndef ENOTSUP
#define ENOTSUP     95  // Operation not supported
#endif

// VFS file type flags
#define VFS_FILE      0x01
#define VFS_DIRECTORY 0x02
#define VFS_CHARDEV   0x04
#define VFS_BLOCKDEV  0x08
#define VFS_PIPE      0x10
#define VFS_SYMLINK   0x20
#define VFS_MOUNTPOINT 0x40

// File mode helpers if not provided by sys/stat.h
#ifndef S_ISREG
#define S_ISREG(m)  (((m) & S_IFMT) == S_IFREG)
#endif
#ifndef S_ISDIR
#define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)
#endif

// Maximum lengths
#define VFS_NAME_MAX 256
#define VFS_PATH_MAX 4096

// File open flags
#define O_RDONLY    0x0000
#define O_WRONLY    0x0001
#define O_RDWR      0x0002
#define O_ACCMODE   0x0003
#define O_CREAT     0x0100
#define O_EXCL      0x0200
#define O_TRUNC     0x0400
#define O_APPEND    0x0800
#define O_DIRECTORY 0x100000

// lseek whence values
#define SEEK_SET    0
#define SEEK_CUR    1
#define SEEK_END    2

// File types - use S_IFMT and S_IFDIR/S_IFREG instead
#define FT_UNKNOWN  0
#define FT_FILE     S_IFREG
#define FT_DIR      S_IFDIR

// File system node structure
typedef struct vfs_node vfs_node_t;

typedef struct vfs_dirent {
    char name[VFS_NAME_MAX];
    uint32_t size;
    int is_dir; // 1 = directory, 0 = file
} vfs_dirent_t;

// File operations function pointers
typedef ssize_t (*read_type_t)(vfs_node_t* node, uint32_t offset, void* buf, size_t count);
typedef ssize_t (*write_type_t)(vfs_node_t* node, uint32_t offset, const void* buf, size_t count);
typedef int (*open_type_t)(vfs_node_t* node, uint32_t flags);
typedef int (*close_type_t)(vfs_node_t* node);
typedef vfs_dirent_t (*readdir_type_t)(vfs_node_t* node, uint32_t index);
typedef int (*finddir_type_t)(vfs_node_t* node, const char* name, vfs_node_t* out_node);
typedef off_t (*lseek_type_t)(vfs_node_t* node, off_t offset, int whence);
typedef int (*stat_type_t)(vfs_node_t* node, struct stat* st);

struct vfs_node {
    char name[VFS_NAME_MAX];
    uint32_t size;          // File size in bytes
    uint32_t flags;         // File flags
    uint32_t inode;         // Inode number
    uint32_t impl;          // Implementation-specific value
    
    // Function pointers
    read_type_t read;
    write_type_t write;
    open_type_t open;
    close_type_t close;
    readdir_type_t readdir;
    finddir_type_t finddir;
    lseek_type_t lseek;
    stat_type_t stat;  // For getting file status
    
    void* priv; // Private data for the filesystem implementation
    void* data; // File data buffer (optional)
    vfs_node_t* parent; // Parent directory node
    vfs_node_t* children; // First child node (for directories)
    vfs_node_t* next; // Next sibling node (for directories)
    uint32_t position; // Current position in file (for read/write operations)
};

// Initialize the virtual file system
void vfs_init(void);

// Mount a filesystem at the given path
int vfs_mount(const char* path, vfs_node_t* fs_root);

// Set the root filesystem
void vfs_set_root(vfs_node_t* root);

// Get the root filesystem node
vfs_node_t* vfs_get_root(void);

// Lookup a file or directory by path
int vfs_lookup(const char* path, vfs_node_t* out_node);

// File operations
int vfs_open(const char* path, int flags);
int vfs_close(int fd);
ssize_t vfs_read(int fd, void* buf, size_t count);
ssize_t vfs_write(int fd, const void* buf, size_t count);
off_t vfs_lseek(int fd, off_t offset, int whence);

// Read helper for files
int vfs_read_all(const char* path, void* buf, uint32_t maxlen, uint32_t* out_len);

// Directory operations
int vfs_opendir(const char* path, vfs_node_t* out_node);
vfs_dirent_t* vfs_read_dir(vfs_node_t* dir);
int vfs_close_dir(vfs_node_t* dir);

// Directory listing helper: fills entries array up to max, returns count
int vfs_list(const char* path, vfs_dirent_t* entries, int max_entries);

// Create a new file or directory
int vfs_create(const char* path, uint32_t flags);
int vfs_mkdir(const char* path);

// Remove a file or directory
int vfs_remove(const char* path);
int vfs_rmdir(const char* path);

// File information
int vfs_stat(const char* path, struct stat* st);

// Helper function to create a basic VFS node
vfs_node_t* vfs_create_node(const char* name, uint32_t flags);

// FAT32 filesystem driver
vfs_node_t* fat32_mount(const char* device);

#endif // _VFS_H
