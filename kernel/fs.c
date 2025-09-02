#include <kernel/fs.h>
#include <kernel/kalloc.h>
#include <kernel/console.h>
#include <string.h>

#define NR_OPEN_DEFAULT 32

static struct file *fd_table[NR_OPEN_DEFAULT] = {0};
static struct file_system_type *file_systems = NULL;

// Initialize file system
void fs_init(void) {
    memset(fd_table, 0, sizeof(fd_table));
    console_puts("File system initialized\n");
}

// Register a filesystem
int register_filesystem(struct file_system_type *fs) {
    if (!fs || !fs->name || !fs->mount) {
        return -1;
    }
    
    fs->next = file_systems;
    file_systems = fs;
    return 0;
}

// Unregister a filesystem
int unregister_filesystem(struct file_system_type *fs) {
    if (!fs) {
        return -1;
    }
    
    struct file_system_type **fs_ptr = &file_systems;
    while (*fs_ptr) {
        if (*fs_ptr == fs) {
            *fs_ptr = fs->next;
            return 0;
        }
        fs_ptr = &(*fs_ptr)->next;
    }
    
    return -1;
}

// Get filesystem type by name
struct file_system_type *get_fs_type(const char *name) {
    struct file_system_type *fs;
    
    for (fs = file_systems; fs; fs = fs->next) {
        if (strcmp(fs->name, name) == 0) {
            return fs;
        }
    }
    
    return NULL;
}

// Get an unused file descriptor
int get_unused_fd(void) {
    for (int i = 0; i < NR_OPEN_DEFAULT; i++) {
        if (!fd_table[i]) {
            return i;
        }
    }
    return -1;
}

// Install a file in the file descriptor table
int fd_install(int fd, struct file *file) {
    if (fd < 0 || fd >= NR_OPEN_DEFAULT) {
        return -1;
    }
    
    if (fd_table[fd]) {
        return -1; // FD already in use
    }
    
    fd_table[fd] = file;
    return 0;
}

// Get file from file descriptor
struct file *fget(int fd) {
    if (fd < 0 || fd >= NR_OPEN_DEFAULT) {
        return NULL;
    }
    
    return fd_table[fd];
}

// Release a file
int fput(struct file *file) {
    if (!file) {
        return -1;
    }
    
    // Call release operation if available
    if (file->f_ops && file->f_ops->release) {
        file->f_ops->release(file->inode, file);
    }
    
    // Free the file structure
    kfree(file);
    return 0;
}

// Close a file descriptor
int sys_close(int fd) {
    if (fd < 0 || fd >= NR_OPEN_DEFAULT) {
        return -1;
    }
    
    struct file *file = fd_table[fd];
    if (!file) {
        return -1;
    }
    
    fd_table[fd] = NULL;
    return fput(file);
}

// Read from a file descriptor
ssize_t sys_read(int fd, void *buf, size_t count) {
    if (fd < 0 || fd >= NR_OPEN_DEFAULT || !fd_table[fd] || !buf) {
        return -1;
    }
    
    struct file *file = fd_table[fd];
    if (!file->f_ops || !file->f_ops->read) {
        return -1;
    }
    
    return file->f_ops->read(file, buf, count, &file->pos);
}

// Write to a file descriptor
ssize_t sys_write(int fd, const void *buf, size_t count) {
    if (fd < 0 || fd >= NR_OPEN_DEFAULT || !fd_table[fd] || !buf) {
        return -1;
    }
    
    struct file *file = fd_table[fd];
    if (!file->f_ops || !file->f_ops->write) {
        return -1;
    }
    
    ssize_t ret = file->f_ops->write(file, buf, count, &file->pos);
    return ret;
}

// Change file position
off_t sys_lseek(int fd, off_t offset, int whence) {
    if (fd < 0 || fd >= NR_OPEN_DEFAULT || !fd_table[fd]) {
        return -1;
    }
    
    struct file *file = fd_table[fd];
    if (!file->f_ops || !file->f_ops->llseek) {
        return -1;
    }
    
    return file->f_ops->llseek(file, offset, whence);
}

// Open a file
int sys_open(const char *filename, int flags, mode_t mode) {
    if (!filename) {
        return -1;
    }
    
    // TODO: Implement actual file opening logic
    // This is a placeholder that just allocates a file descriptor
    
    struct file *file = kmalloc(sizeof(struct file));
    if (!file) {
        return -1;
    }
    
    memset(file, 0, sizeof(struct file));
    
    int fd = get_unused_fd();
    if (fd < 0) {
        kfree(file);
        return -1;
    }
    
    file->fd = fd;
    file->flags = flags;
    file->pos = 0;
    
    if (fd_install(fd, file) < 0) {
        kfree(file);
        return -1;
    }
    
    return fd;
}
