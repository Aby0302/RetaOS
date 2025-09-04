#include "include/kernel/vfs.h"
#include <string.h>
#include "include/memory/heap.h"
#include "include/kernel/console.h"
#include "include/kernel/console_utils.h"  // For console_printf
#include "fs/fat32_vfs.h"
#include "include/drivers/serial.h"
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <time.h>  // For struct timespec and clock_gettime

// Custom implementation of strtok_r for freestanding environment
char* strtok_r(char* str, const char* delim, char** saveptr) {
    char* token;
    
    if (str == NULL) {
        str = *saveptr;
    }
    
    // Skip leading delimiters
    str += strspn(str, delim);
    if (*str == '\0') {
        *saveptr = str;
        return NULL;
    }
    
    // Find the end of the token
    token = str;
    str = strpbrk(token, delim);
    if (str == NULL) {
        *saveptr = (char*)token + strlen(token);
    } else {
        *str = '\0';
        *saveptr = str + 1;
    }
    
    return token;
}

// Define ENOTEMPTY if not already defined
#ifndef ENOTEMPTY
#define ENOTEMPTY 39  // Directory not empty
#endif



// Maximum number of open files
#define MAX_OPEN_FILES 32

// File descriptor table
static vfs_node_t open_files[MAX_OPEN_FILES] = {0};

// Root filesystem
static vfs_node_t vfs_root = {0};


void vfs_init(void) {
    // Initialize the VFS root
    memset(&vfs_root, 0, sizeof(vfs_node_t));
    strncpy(vfs_root.name, "/", VFS_NAME_MAX - 1);
    vfs_root.name[VFS_NAME_MAX - 1] = '\0';
    vfs_root.flags = S_IFDIR | 0755;  // Directory with rwxr-xr-x permissions
    vfs_root.size = 0;
    vfs_root.position = 0;
    vfs_root.children = NULL;
    vfs_root.next = NULL;
    vfs_root.parent = NULL;
    
    // Initialize file descriptor table
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        memset(&open_files[i], 0, sizeof(vfs_node_t));
    }
    
    // Mount the root filesystem (FAT32)
    vfs_node_t* root = fat32_mount("hd0");
    if (root && root->name[0] != '\0') { // Check if root is valid
        // Mount the root filesystem at "/"
        if (vfs_mount("/", root) == 0) {
            console_printf("VFS: FAT32 filesystem mounted at /\n");
        } else {
            console_printf("VFS: Failed to mount root filesystem at /\n");
        }
        kfree(root); // Free the temporary root node
    } else {
        console_printf("VFS: Failed to initialize FAT32 filesystem!\n");
        if (root) {
            kfree(root); // Free the failed root node
        }
    }
    
    // Create essential directories if they don't exist
    vfs_mkdir("/dev");
    vfs_mkdir("/proc");
    vfs_mkdir("/tmp");
    
    console_printf("VFS: Initialization complete\n");
}

int vfs_mount(const char* path, vfs_node_t* fs_root) {
    if (!path || !fs_root || !fs_root->name[0]) {
        return -EINVAL;
    }
    
    // Look up the mount point
    vfs_node_t mount_point;
    int ret = vfs_lookup(path, &mount_point);
    if (ret != 0) {
        // If the mount point doesn't exist, try to create parent directories
        char parent_path[VFS_PATH_MAX] = {0};
        const char* last_slash = strrchr(path, '/');
        
        if (last_slash && last_slash != path) {
            // Extract parent directory path
            size_t parent_len = last_slash - path;
            if (parent_len >= VFS_PATH_MAX) {
                return -ENAMETOOLONG;
            }
            
            strncpy(parent_path, path, parent_len);
            parent_path[parent_len] = '\0';
            
            // Look up parent directory
            vfs_node_t parent;
            ret = vfs_lookup(parent_path, &parent);
            if (ret != 0) {
                return ret;
            }
            if (!S_ISDIR(parent.flags)) {
                return -ENOTDIR;
            }
        }
        
        // Create the mount point directory
        return vfs_mkdir(path);
    }
    
    // Check if the mount point is a directory
    if (!S_ISDIR(mount_point.flags)) {
        return -ENOTDIR;
    }
    
    // If mounting at the global root '/', replace the global vfs_root
    if (strcmp(path, "/") == 0) {
        vfs_set_root(fs_root);
        // If the mounted filesystem has an open function, call it on the real root
        if (vfs_root.open) {
            ret = vfs_root.open(&vfs_root, 0);
            if (ret != 0) {
                return ret;
            }
        }
        {
            serial_write("VFS: Mounted root set from fat32 (root name='");
            serial_write(vfs_root.name);
            serial_write("')\n");
        }
        return 0;
    }

    // Otherwise, copy the root node's data to the mount point (for non-root mounts)
    memcpy(&mount_point, fs_root, sizeof(vfs_node_t));

    // If the mounted filesystem has an open function, call it
    if (mount_point.open) {
        ret = mount_point.open(&mount_point, 0);
        if (ret != 0) {
            return ret;
        }
    }

    return 0; // Success
}

void vfs_set_root(vfs_node_t* root) {
    if (!root) {
        return;
    }
    
    // Copy the root node's data to the global root
    memcpy(&vfs_root, root, sizeof(vfs_node_t));
}

vfs_node_t* vfs_get_root(void) {
    return &vfs_root;
}

// Commented out unused function
/*
static int is_sep(char c) {
    return c == '/' || c == '\\';
}
*/

static int child_lookup(vfs_node_t* dir, const char* name, vfs_node_t* out_node) {
    if (!dir || !out_node || !(dir->flags & S_IFDIR)) {
        return -ENOTDIR;
    }
    
    vfs_node_t* child = dir->children;
    while (child) {
        if (strcmp(child->name, name) == 0) {
            memcpy(out_node, child, sizeof(vfs_node_t));
            return 0;
        }
        child = child->next;
    }
    return -ENOENT;
}

int vfs_lookup(const char* path, vfs_node_t* out_node) {
    if (!path || !out_node) {
        return -EINVAL;
    }
    
    // Start from the root node
    memcpy(out_node, &vfs_root, sizeof(vfs_node_t));
    
    // Handle empty path or root path
    if (path[0] == '\0' || (path[0] == '/' && path[1] == '\0')) {
        return 0;
    }
    
    // Make a copy of the path to work with
    char path_copy[VFS_PATH_MAX];
    strncpy(path_copy, path, sizeof(path_copy) - 1);
    path_copy[sizeof(path_copy) - 1] = '\0';
    
    // Handle absolute paths
    char* token = path_copy;
    if (path_copy[0] == '/') {
        token++;
    }
    
    // Tokenize the path
    char* saveptr;
    char* component = strtok_r(token, "/", &saveptr);
    if (component) {
        serial_write("VFS: lookup path='"); serial_write(path); serial_write("' start component='"); serial_write(component); serial_write("'\n");
    } else {
        serial_write("VFS: lookup path has no components\n");
    }
    
    vfs_node_t current_node;
    memcpy(&current_node, out_node, sizeof(vfs_node_t));
    
    while (component != NULL) {
        // Handle . and ..
        if (strcmp(component, ".") == 0) {
            // Do nothing, stay in current directory
            component = strtok_r(NULL, "/", &saveptr);
            continue;
        } else if (strcmp(component, "..") == 0) {
            // Move to parent directory if possible
            if (current_node.parent) {
                memcpy(&current_node, current_node.parent, sizeof(vfs_node_t));
            }
            // If no parent, we're already at root
            component = strtok_r(NULL, "/", &saveptr);
            continue;
        }
        
        // Look for the component in the current directory
        vfs_node_t child = {0};
        int ret = -ENOENT;
        
        // First, check if the current node has a finddir function
        if (current_node.finddir) {
            ret = current_node.finddir(&current_node, component, &child);
        }
        
        // If not found, check the children list
        if (ret != 0) {
            vfs_node_t* node = current_node.children;
            while (node) {
                if (strcmp(node->name, component) == 0) {
                    memcpy(&child, node, sizeof(vfs_node_t));
                    ret = 0;
                    break;
                }
                node = node->next;
            }
        }
        
        // If still not found, return error
        if (ret != 0) {
            serial_write("VFS: lookup failed at component='"); serial_write(component); serial_write("' for path='"); serial_write(path); serial_write("'\n");
            return -ENOENT;
        }
        
        // Update current node to the found child
        memcpy(&current_node, &child, sizeof(vfs_node_t));
        
        // Get next component
        component = strtok_r(NULL, "/", &saveptr);
        
        // If this is a symlink, resolve it
        if (S_ISLNK(current_node.flags)) { // Use standard macro
            char link_target[VFS_PATH_MAX] = {0};
            
            // Read the link target
            if (current_node.read) {
                ssize_t bytes_read = current_node.read(&current_node, 0, link_target, sizeof(link_target) - 1);
                if (bytes_read > 0) {
                    link_target[bytes_read] = '\0';
                    
                    // If there are more components, append them to the link target
                    if (component) {
                        size_t len = strlen(link_target);
                        if (len + 1 < sizeof(link_target)) {
                            link_target[len] = '/';
                            strncpy(link_target + len + 1, component, sizeof(link_target) - len - 2);
                            link_target[sizeof(link_target) - 1] = '\0';
                            
                            // Skip the component we just appended
                            component = strtok_r(NULL, "/", &saveptr);
                        }
                    }
                    
                    // Look up the link target with the current path components
                    return vfs_lookup(link_target, out_node);
                }
            }
            return -EIO; // Error reading symlink
        }
    }
    // Return the final resolved node
    memcpy(out_node, &current_node, sizeof(vfs_node_t));
    return 0;
}

// Read data from a file
ssize_t vfs_read(int fd, void* buf, size_t count) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !open_files[fd].name[0]) {
        return -EBADF;
    }
    
    if (!buf) {
        return -EINVAL;
    }
    
    vfs_node_t* node = &open_files[fd];
    
    // Check if the node is a directory
    if (node->flags & FT_DIR) {
        return -EISDIR;
    }
    
    // Check if reading is allowed
    if (!node->read && !node->data) {
        return -EBADF; 
    }
    
    // If at end of file, return 0
    if (node->position >= node->size) {
        return 0;
    }
    
    // Calculate how much we can read
    size_t to_read = count;
    if (node->position + to_read > node->size) {
        to_read = node->size - node->position;
    }
    
    // If there's a read method, use it
    if (node->read) {
        ssize_t result = node->read(node, node->position, buf, to_read);
        if (result > 0) {
            node->position += result;
        }
        return result;
    }
    
    // Otherwise, read from the data buffer if it exists
    if (node->data) {
        memcpy(buf, (char*)node->data + node->position, to_read);
        node->position += to_read;
        return to_read;
    }
    
    return -EIO; // Shouldn't reach here
}

// Write data to a file
ssize_t vfs_write(int fd, const void* buf, size_t count) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !open_files[fd].name[0]) {
        return -EBADF;
    }
    
    if (!buf) {
        return -EINVAL;
    }
    
    vfs_node_t* node = &open_files[fd];
    
    // Check if the node is a directory
    if (node->flags & FT_DIR) {
        return -EISDIR;
    }
    
    // Check if writing is allowed
    if (!node->write && !(node->flags & O_APPEND)) {
        return -EBADF;
    }
    
    // If there's a write method, use it
    if (node->write) {
        ssize_t result = node->write(node, node->position, buf, count);
        if (result > 0) {
            node->position += result;
            if (node->position > node->size) {
                node->size = node->position;
            }
        }
        return result;
    }
    
    // Handle in-memory files
    if (node->data) {
        // Resize the buffer if needed
        if (node->position + count > node->size) {
            void* new_data = kmalloc(node->position + count);
            if (!new_data) {
                return -ENOMEM;
            }
            if (node->data) {
                memcpy(new_data, node->data, node->size);
                kfree(node->data);
            }
            node->data = new_data;
            node->size = node->position + count;
        }
        
        // Copy the data
        memcpy((char*)node->data + node->position, buf, count);
        node->position += count;
        return count;
    }
    
    return -EROFS; // Read-only filesystem
}

// Open a file
int vfs_open(const char* path, int flags) {
    if (!path) {
        return -EINVAL;
    }
    
    // Find a free file descriptor
    int fd = -1;
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (!open_files[i].name[0]) {
            fd = i;
            break;
        }
    }
    
    if (fd == -1) {
        return -EMFILE; // Too many open files
    }
    
    // Look up the file
    vfs_node_t node;
    int ret = vfs_lookup(path, &node);
    
    // Handle file creation if needed
    if (ret != 0) {
        if (!(flags & O_CREAT)) {
            return -ENOENT; // File doesn't exist and we're not creating it
        }
        
        // Extract directory path and filename
        char dir_path[VFS_PATH_MAX] = {0};
        char file_name[VFS_NAME_MAX] = {0};
        
        const char* last_slash = strrchr(path, '/');
        if (!last_slash) {
            // No directory component, use current directory
            strncpy(file_name, path, VFS_NAME_MAX - 1);
            strncpy(dir_path, ".", VFS_PATH_MAX - 1);
        } else if (last_slash == path) {
            // Root directory
            strncpy(file_name, path + 1, VFS_NAME_MAX - 1);
            strncpy(dir_path, "/", VFS_PATH_MAX - 1);
        } else {
            // Extract directory and filename
            size_t dir_len = last_slash - path;
            if (dir_len >= VFS_PATH_MAX) {
                return -ENAMETOOLONG;
            }
            strncpy(dir_path, path, dir_len);
            dir_path[dir_len] = '\0';
            strncpy(file_name, last_slash + 1, VFS_NAME_MAX - 1);
        }
        
        // Look up the parent directory
        vfs_node_t parent_dir;
        ret = vfs_lookup(dir_path, &parent_dir);
        if (ret != 0) {
            return -ENOENT; // Parent directory doesn't exist
        }
        
        // Check if parent is a directory
        if (!(parent_dir.flags & FT_DIR)) {
            return -ENOTDIR; // Parent is not a directory
        }
        
        // Check write permissions on the parent directory
        if (!(parent_dir.flags & 0222)) {
            return -EACCES; // No write permission on parent directory
        }
        
        // Create the file
        ret = vfs_create(path, 0666); // Default permissions: rw-rw-rw-
        if (ret != 0) {
            return ret;
        }
        
        // Look it up again
        ret = vfs_lookup(path, &node);
        if (ret != 0) {
            return ret;
        }
    }
    
    // Check if it's a directory
    if ((node.flags & FT_DIR) && (flags & (O_WRONLY | O_RDWR))) {
        return -EISDIR; // Can't open directory for writing
    }
    
    // Check access mode
    int access_mode = flags & O_ACCMODE;
    
    // Check permissions based on access mode
    switch (access_mode) {
        case O_RDONLY:
            if (!(node.flags & 0444)) {
                return -EACCES; // No read permission
            }
            break;
            
        case O_WRONLY:
            if (!(node.flags & 0222)) {
                return -EACCES; // No write permission
            }
            break;
            
        case O_RDWR:
            if (!(node.flags & 0666)) {
                return -EACCES; // No read/write permission
            }
            break;
    }
    
    // Check if the file exists and O_EXCL is set
    if ((flags & O_EXCL) && (flags & O_CREAT) && ret == 0) {
        return -EEXIST; // File exists and O_EXCL is set
    }
    
    // Truncate if needed (only for write modes)
    if ((flags & O_TRUNC) && (access_mode != O_RDONLY)) {
        if (node.flags & O_TRUNC) {
        // Handle truncate by setting size to 0
        node.size = 0;
            if (ret != 0) {
                return ret;
            }
        } else if (node.write) {
            // If no truncate function, try to write an empty buffer
            char empty = 0;
            ret = node.write(&node, 0, &empty, 0);
            if (ret < 0) {
                return ret;
            }
        }
    }
    
    // Initialize the open file
    open_files[fd] = node;
    
    // Set initial position
    if (flags & O_APPEND) {
        open_files[fd].position = node.size; // Start at end of file
    } else {
        open_files[fd].position = 0; // Start at beginning of file
    }
    
    // Call the filesystem-specific open function if it exists
    if (open_files[fd].open) {
        ret = open_files[fd].open(&open_files[fd], flags);
        if (ret != 0) {
            open_files[fd] = (vfs_node_t){0};
            return ret;
        }
    }
    
    return fd;
}

// Close an open file
int vfs_close(int fd) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !open_files[fd].name[0]) {
        return -EBADF; // Invalid file descriptor
    }
    
    // Call the filesystem-specific close function if it exists
    int ret = 0;
    if (open_files[fd].close) {
        ret = open_files[fd].close(&open_files[fd]);
    }
    
    // Clear the file descriptor slot
    open_files[fd] = (vfs_node_t){0};
    
    return ret;
}

// Change the file position
off_t vfs_lseek(int fd, off_t offset, int whence) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !open_files[fd].name[0]) {
        return -EBADF;
    }
    
    vfs_node_t* node = &open_files[fd];
    off_t new_pos;
    
    switch (whence) {
        case SEEK_SET:
            new_pos = offset;
            break;
            
        case SEEK_CUR:
            new_pos = node->position + offset;
            break;
            
        case SEEK_END:
            new_pos = node->size + offset;
            break;
            
        default:
            return -EINVAL;
    }
    
    // Check for negative positions
    if (new_pos < 0) {
        return -EINVAL;
    }
    
    // If the file has a custom lseek handler, use it
    if (node->lseek) {
        return node->lseek(node, offset, whence);
    }
    
    // Update the position
    node->position = new_pos;
    
    return node->position;
}

int vfs_size(int fd) {
    if (fd < 0 || fd >= MAX_OPEN_FILES || !open_files[fd].name[0]) {
        return -EBADF;
    }
    return (int)open_files[fd].size;
}

// Read helper for files
int vfs_read_all(const char* path, void* buf, uint32_t maxlen, uint32_t* out_len) {
    if (!path || !buf || maxlen == 0) {
        return -EINVAL;
    }
    
    int fd = vfs_open(path, O_RDONLY);
    if (fd < 0) {
        return fd;
    }
    
    ssize_t bytes_read = vfs_read(fd, buf, maxlen - 1); // Leave space for null terminator
    int close_ret = vfs_close(fd);
    
    if (bytes_read < 0) {
        return (int)bytes_read;
    }
    
    // Null-terminate the buffer
    ((char*)buf)[bytes_read] = '\0';
    
    if (out_len) {
        *out_len = (uint32_t)bytes_read;
    }
    
    // If close failed but read succeeded, still return success
    return (close_ret < 0) ? close_ret : 0;
}

int vfs_list(const char* path, vfs_dirent_t* entries, int max_entries) {
    if (!path || !entries || max_entries <= 0) {
        return -EINVAL;
    }
    
    vfs_node_t dir;
    int ret = vfs_lookup(path, &dir);
    if (ret != 0) {
        return ret;
    }
    
    if (!(dir.flags & FT_DIR)) {
        return -ENOTDIR;
    }
    
    int count = 0;
    vfs_node_t* child = dir.children;
    
    while (child && count < max_entries) {
        // Skip invalid entries
        if (!child->name[0]) {
            child = child->next;
            continue;
        }
        
        // Copy entry information
        strncpy(entries[count].name, child->name, VFS_NAME_MAX - 1);
        entries[count].name[VFS_NAME_MAX - 1] = '\0';
        entries[count].size = child->size;
        entries[count].is_dir = (child->flags & FT_DIR) ? 1 : 0;
        
        count++;
        child = child->next;
    }
    
    return count;
}

// Directory operations
int vfs_opendir(const char* path, vfs_node_t* out_dir) {
    if (!path || !out_dir) {
        return -EINVAL;
    }
    
    int ret = vfs_lookup(path, out_dir);
    if (ret != 0) {
        return ret;
    }
    
    if (!(out_dir->flags & FT_DIR)) {
        return -ENOTDIR;
    }
    
    // Reset position for directory reading
    out_dir->position = 0;
    
    return 0;
}

vfs_dirent_t* vfs_readdir(vfs_node_t* dir, vfs_dirent_t* entry) {
    if (!dir || !entry) {
        // Set errno and return NULL
        errno = EINVAL;
        return NULL;
    }
    
    if (!(dir->flags & FT_DIR)) {
        // Set errno and return NULL
        errno = ENOTDIR;
        return NULL;
    }
    
    // If there's a readdir function, use it
    if (dir->readdir) {
        *entry = dir->readdir(dir, dir->position);
        if (entry->name[0] == '\0') {
            // Set errno and return NULL
            errno = ENOENT; // End of directory
            return NULL;
        }
        dir->position++;
        return entry;
    }
    
    // Otherwise, use the children list
    vfs_node_t* child = dir->children;
    uint32_t i = 0;
    
    // Skip to the current position
    while (child && i < dir->position) {
        child = child->next;
        i++;
    }
    
    if (!child) {
        // Set errno and return NULL
        errno = ENOENT; // End of directory
        return NULL;
    }
    
    // Fill in the entry
    strncpy(entry->name, child->name, VFS_NAME_MAX - 1);
    entry->name[VFS_NAME_MAX - 1] = '\0';
    entry->size = child->size;
    entry->is_dir = (child->flags & FT_DIR) ? 1 : 0;
    
    // Move to next position
    dir->position++;
    
    return entry;
}

vfs_dirent_t* vfs_read_dir(vfs_node_t* dir) {
    static vfs_dirent_t entry;
    static uint32_t index = 0;
    
    if (!dir || !(dir->flags & FT_DIR)) {
        entry.name[0] = '\0';
        return &entry;
    }
    
    if (dir->readdir) {
        entry = dir->readdir(dir, index++);
        return &entry;
    }
    
    entry.name[0] = '\0';
    return &entry;
}

int vfs_closedir(vfs_node_t* dir) {
    if (!dir) {
        return -EBADF;
    }
    
    // Reset the position for future use
    dir->position = 0;
    return 0;
}

// Create a new file or directory
int vfs_create(const char* path, uint32_t flags) {
    if (!path || *path == '\0') {
        return -EINVAL;
    }
    
    // Extract the parent directory path
    char parent_path[VFS_PATH_MAX];
    const char* name = strrchr(path, '/');
    
    if (!name) {
        // No parent directory, use current directory
        strncpy(parent_path, ".", sizeof(parent_path) - 1);
        name = path;
    } else if (name == path) {
        // Path starts with '/', parent is root
        strncpy(parent_path, "/", sizeof(parent_path) - 1);
        name++;
    } else {
        // Copy parent path
        size_t parent_len = name - path;
        if (parent_len >= sizeof(parent_path)) {
            return -ENAMETOOLONG;
        }
        strncpy(parent_path, path, parent_len);
        parent_path[parent_len] = '\0';
        name++; // Skip the '/'
    }
    
    // Look up the parent directory
    vfs_node_t* parent = kmalloc(sizeof(vfs_node_t));
    if (!parent) {
        return -ENOMEM;
    }
    
    int ret = vfs_lookup(parent_path, parent);
    if (ret != 0) {
        kfree(parent);
        return ret;
    }
    // Ensure cleanup in all error paths
    
    // Check if parent is a directory
    if (!(parent->flags & FT_DIR)) {
        kfree(parent);
        return -ENOTDIR;
    }
    
    // Check if the file already exists
    vfs_node_t existing;
    if (vfs_lookup(path, &existing) == 0) {
        kfree(parent);
        return -EEXIST;
    }
    
    // Create a new node
    vfs_node_t* new_node_ptr = vfs_create_node(name, flags);
    if (!new_node_ptr) {
        kfree(parent);
        return -ENOMEM;
    }
    if (!new_node_ptr) {
        kfree(parent);
        return -ENOMEM;
    }
    
    // Add to parent's children
    new_node_ptr->parent = parent;
    new_node_ptr->next = parent->children;
    parent->children = new_node_ptr;
    
    // Clean up
    kfree(parent);
    
    return 0; // Success
}

// File information
int vfs_stat(const char* path, struct stat* st) {
    if (!path || !st) {
        return -EINVAL;
    }
    
    vfs_node_t node;
    int ret = vfs_lookup(path, &node);
    if (ret != 0) {
        return ret;
    }
    
    // Clear the stat structure
    memset(st, 0, sizeof(struct stat));
    
    // Fill in basic information
    st->st_size = node.size;
    st->st_mode = node.flags;
    
    st->st_mode = node.flags; // Use existing flags directly
    
    // Set timestamps to current time if not available
    // Using simpler time handling for now
    st->st_atime = 0;  // Last access time
    st->st_mtime = 0;  // Last modification time
    st->st_ctime = 0;  // Last status change time
    
    // Using simpler stat implementation for now
    // Filesystem implementations can override these values if needed
    
    return 0;  // Success
}

// Helper function to create a basic VFS node
vfs_node_t* vfs_create_node(const char* name, uint32_t flags) {
    vfs_node_t* node = (vfs_node_t*)kmalloc(sizeof(vfs_node_t));
    if (!node) {
        return NULL;
    }
    
    // Initialize all fields to zero
    memset(node, 0, sizeof(vfs_node_t));
    
    // Copy the name if provided
    if (name) {
        strncpy(node->name, name, VFS_NAME_MAX - 1);
        node->name[VFS_NAME_MAX - 1] = '\0';
    } else {
        node->name[0] = '\0';
    }
    
    // Set flags and initialize other fields
    node->flags = flags;
    node->size = 0;
    node->position = 0;
    node->data = NULL;
    node->children = NULL;
    node->next = NULL;
    node->parent = NULL;
    node->priv = NULL;
    
    return node;
}

// Create a new directory
int vfs_mkdir(const char* path) {
    if (!path || *path == '\0') {
        return -EINVAL;
    }
    
    // Create with directory flag
    return vfs_create(path, S_IFDIR | 0755);
}

// Remove a file
int vfs_remove(const char* path) {
    if (!path || *path == '\0') {
        return -EINVAL;
    }
    
    vfs_node_t node;
    int ret = vfs_lookup(path, &node);
    if (ret != 0) {
        return ret;
    }
    
    // Check if it's a directory
    if (S_ISDIR(node.flags)) {
        return -EISDIR;
    }
    
    // If there's a close function, call it
    if (node.close) {
        node.close(&node);
    }
    
    // Remove from parent's children list
    if (node.parent) {
        vfs_node_t** child_ptr = &node.parent->children;
        while (*child_ptr) {
            if ((*child_ptr)->name == node.name) { // Compare by name instead of pointer
                vfs_node_t* to_remove = *child_ptr;
                *child_ptr = to_remove->next;
                kfree(to_remove->priv);
                kfree(to_remove);
                return 0;
            }
            child_ptr = &(*child_ptr)->next;
        }
    }
    
    return -ENOENT; // Shouldn't happen
}

// Remove a directory
int vfs_rmdir(const char* path) {
    if (!path || *path == '\0') {
        return -EINVAL;
    }
    
    vfs_node_t node;
    int ret = vfs_lookup(path, &node);
    if (ret != 0) {
        return ret;
    }
    
    // Check if it's a directory
    if (!S_ISDIR(node.flags)) {
        return -ENOTDIR;
    }
    
    // Check if directory is empty
    vfs_node_t dir_node;
    ret = vfs_opendir(path, &dir_node);
    if (ret != 0) {
        return -EIO;
    }
    
    vfs_dirent_t entry;
    int has_entries = 0;
    while (vfs_readdir(&dir_node, &entry) != NULL) {
        if (strcmp(entry.name, ".") != 0 && strcmp(entry.name, "..") != 0) {
            has_entries = 1;
            break;
        }
    }
    vfs_closedir(&dir_node);
    if (has_entries) return -ENOTEMPTY;
    
    vfs_closedir(&dir_node);
    
    // If there's a close function, call it
    if (node.close) {
        node.close(&node);
    }
    
    // Remove from parent's children list
    if (node.parent) {
        vfs_node_t** child_ptr = &node.parent->children;
        while (*child_ptr) {
            vfs_node_t* current_node = *child_ptr;
            if (*child_ptr == &node) {
                *child_ptr = current_node->next;
                kfree(current_node->priv);
                kfree(current_node);
                return 0;
            }
            child_ptr = &(*child_ptr)->next;
        }
    }
    
    return -ENOENT; // Shouldn't happen
}

// FAT32 filesystem driver - moved to fs/fat32.c
// vfs_node_t* fat32_mount(const char* device) {
//     // This is a stub - implement actual FAT32 mounting logic here
//     vfs_node_t* node = vfs_create_node("fat32_root", S_IFDIR | 0755);
//     if (!node) {
//         return NULL;
//     }
//     return node;
// }
