#ifndef _KERNEL_FS_H
#define _KERNEL_FS_H

#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h> // Use libc's file mode and permission macros
#include <dirent.h>   // Use libc's DIR and struct dirent

// File descriptor table entry
struct file {
    int fd;                 // File descriptor
    int flags;              // File status flags
    off_t pos;              // Current position
    struct inode *inode;    // Pointer to inode
    struct file_ops *f_ops; // File operations
};

// Inode structure
struct inode {
    uint32_t i_ino;         // Inode number
    uint16_t i_mode;        // File type and mode
    uint16_t i_uid;         // User ID of owner
    uint16_t i_gid;         // Group ID of owner
    uint32_t i_size;        // Size in bytes
    uint32_t i_blocks;      // Number of blocks
    uint32_t i_atime;       // Access time
    uint32_t i_mtime;       // Modification time
    uint32_t i_ctime;       // Creation time
    void *i_private;        // Filesystem private data
};

// File operations structure
struct file_ops {
    ssize_t (*read)(struct file *file, char *buf, size_t count, off_t *offset);
    ssize_t (*write)(struct file *file, const char *buf, size_t count, off_t *offset);
    int (*open)(struct inode *inode, struct file *file);
    int (*release)(struct inode *inode, struct file *file);
    off_t (*llseek)(struct file *file, off_t offset, int whence);
};

// File system operations
struct file_system_type {
    const char *name;
    int (*mount)(const char *source, const char *target, const char *type, unsigned long flags, const void *data);
    struct file_system_type *next;
};

// File system related functions
int register_filesystem(struct file_system_type *fs);
int unregister_filesystem(struct file_system_type *fs);
struct file_system_type *get_fs_type(const char *name);

// File operations
struct file *fget(int fd);
int fput(struct file *file);
int fd_install(int fd, struct file *file);
int get_unused_fd(void);

#endif // _KERNEL_FS_H
