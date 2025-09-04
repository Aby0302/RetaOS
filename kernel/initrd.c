#include <kernel/initrd.h>
#include <kernel/vfs.h>
#include <kernel/block.h>
#include <kernel/fs.h>
#include <drivers/serial.h>
#include <memory/heap.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>

// File operations function declarations
static ssize_t tar_read(vfs_node_t* node, uint32_t offset, void* buf, size_t count);
static int tar_open(vfs_node_t* node, uint32_t flags);
static int tar_close(vfs_node_t* node);
static vfs_dirent_t tar_readdir(vfs_node_t* node, uint32_t index);
static int tar_finddir(vfs_node_t* node, const char* name, vfs_node_t** out_node);

// VFS operation handlers
static struct vfs_ops tar_ops = {
    .read = tar_read,
    .write = NULL,
    .open = tar_open,
    .close = tar_close,
    .readdir = tar_readdir,
    .finddir = tar_finddir
};

// File operations table
static const struct vfs_file_ops tar_file_ops = {
    .read = tar_read,
    .write = NULL,
    .open = tar_open,
    .close = tar_close,
    .readdir = NULL,
    .finddir = NULL
};

// Keep a reference to the initrd image so VFS file nodes can point into it
static uint8_t* g_initrd_img = NULL;
static uint32_t g_initrd_bytes = 0;

// Ensure a directory path exists under base, creating nodes as needed.
// Returns the vfs_node_t* for the final directory.
static vfs_node_t* ensure_dir(vfs_node_t* base, const char* dirpath) {
    if (!base || !dirpath || !*dirpath) return base;
    char tmp[VFS_PATH_MAX];
    strncpy(tmp, dirpath, sizeof(tmp)-1);
    tmp[sizeof(tmp)-1] = '\0';
    // strip leading './'
    char* p = tmp;
    if (p[0]=='.' && p[1]=='/') p+=2;
    // strip trailing '/'
    size_t len = strlen(p);
    while (len>0 && p[len-1]=='/') { p[--len] = '\0'; }
    if (len==0) return base;

    vfs_node_t* cur = base;
    char* s = p;
    while (*s) {
        // skip redundant separators
        while (*s=='/') s++;
        if (!*s) break;
        // find end of component
        char* start = s;
        while (*s && *s!='/') s++;
        size_t clen = (size_t)(s - start);
        if (clen == 0) continue;
        char comp[VFS_NAME_MAX];
        if (clen >= sizeof(comp)) clen = sizeof(comp)-1;
        memcpy(comp, start, clen);
        comp[clen] = '\0';

        // find existing child dir named comp
        vfs_node_t* ch = cur->children;
        while (ch) {
            if ((ch->flags & S_IFDIR) && strcmp(ch->name, comp)==0) break;
            ch = ch->next;
        }
        if (!ch) {
            ch = kmalloc(sizeof(vfs_node_t));
            if (!ch) return cur; // OOM: return best-effort current dir
            memset(ch, 0, sizeof(vfs_node_t));
            strncpy(ch->name, comp, sizeof(ch->name)-1);
            ch->flags = S_IFDIR | 0755;
            ch->open = tar_open;
            ch->close = tar_close;
            ch->readdir = tar_readdir;
            ch->finddir = tar_finddir;
            ch->parent = cur;
            vfs_node_add_child(cur, ch);
        }
        cur = ch;
        // continue loop; s is at '/' or '\0'
    }
    return cur;
}

// TAR header structure
struct tar_header {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];     // ASCII octal
    char mtime[12];
    char chksum[8];
    char typeflag;     // '0' or '\0' => regular file; '5' => directory
    char linkname[100];
    char magic[6];     // "ustar\0"
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];  // Path prefix
    char pad[12];      // Padding to 512 bytes
};

static inline size_t tar_parse_octal(const char *s, size_t n) {
    size_t v = 0;
    for (size_t i = 0; i < n && s[i]; i++) {
        if (s[i] < '0' || s[i] > '7') break;
        v = (v << 3) + (size_t)(s[i] - '0');  // *** base-8 ***
    }
    return v;
}

static inline int tar_is_reg(char tf) { return (tf == '0' || tf == '\0'); }

static const char* norm_name(const char *nm) {
    if (nm[0] == '.' && nm[1] == '/') return nm + 2;  // Strip "./"
    return nm;
}

typedef struct {
    size_t size;
    uint32_t data_off;   // Offset of file payload within TAR image
} tar_file_meta_t;

static ssize_t tar_read_ram(vfs_node_t *ino, void *buf, size_t off, size_t len);

// Helper functions
static uint32_t parse_octal(const char* str, size_t len) {
    uint32_t val = 0;
    size_t i;
    for (i = 0; i < len && str[i]; i++) {
        if (str[i] < '0' || str[i] > '7') break;
        val = val * 8 + (str[i] - '0');
    }
    return val;
}

// VFS operations implementation
static ssize_t tar_read(vfs_node_t* node, uint32_t offset, void* buf, size_t count) {
    tar_file_meta_t* meta = (tar_file_meta_t*)node->data;
    
    if (!meta || !g_initrd_img) {
        return -EIO;
    }
    
    if (offset >= meta->size) {
        return 0;  // EOF
    }
    
    size_t remaining = meta->size - offset;
    size_t to_read = count < remaining ? count : remaining;
    
    memcpy(buf, g_initrd_img + meta->data_off + offset, to_read);
    return to_read;
}

static int tar_open(vfs_node_t* node, uint32_t flags) {
    // No special handling needed for now
    (void)node;
    (void)flags;
    return 0;
}

static int tar_close(vfs_node_t* node) {
    // No special handling needed for now
    (void)node;
    return 0;
}

static vfs_dirent_t tar_readdir(vfs_node_t* node, uint32_t index) {
    vfs_dirent_t dir;
    memset(&dir, 0, sizeof(dir));
    // Directory listing not implemented yet
    (void)node;
    (void)index;
    return dir;
}

static int tar_finddir(vfs_node_t* node, const char* name, vfs_node_t** out_node) {
    // tar_file_meta_t* m = (tar_file_meta_t*)node->priv; // Commented out unused variable
    // Directory lookup not implemented yet
    (void)node;
    (void)name;
    (void)out_node;
    return -ENOTDIR;
}

// Mount the initial ramdisk
int mount_initrd(uint8_t* img, size_t bytes) {
    if (!img || bytes < 512) {
        return -EINVAL;
    }
    
    g_initrd_img = img;
    g_initrd_bytes = bytes;
    
    // Create root directory node
    vfs_node_t* root = kmalloc(sizeof(vfs_node_t));
    if (!root) {
        return -ENOMEM;
    }
    
    // Initialize root directory
    memset(root, 0, sizeof(vfs_node_t));
    strcpy(root->name, "/");
    root->flags = S_IFDIR | 0755; // directory with default perms
    // Set direct function pointers for directory operations where applicable
    root->read = NULL;
    root->write = NULL;
    root->open = tar_open;
    root->close = tar_close;
    root->readdir = tar_readdir;
    root->finddir = tar_finddir;
    
    // Declare and initialize off and skip variables
    uint32_t off = 0;
    // uint32_t skip = 0; // Commented out unused variable
    
    while (off + 512 <= bytes) {
        const struct tar_header* hdr = (const struct tar_header*)(img + off);
        if (hdr->name[0] == '\0') {
            break;  // End of archive
        }
        
        uint32_t size = parse_octal(hdr->size, sizeof(hdr->size));
        uint32_t mode = parse_octal(hdr->mode, sizeof(hdr->mode)) & 0777; // permissions from TAR
        const char* path = hdr->name;
        
        if (hdr->typeflag == '0' || hdr->typeflag == '\0') {
            // Regular file: place under its parent directory
            // Normalize path
            const char* pth = path;
            if (pth[0]=='.' && pth[1]=='/') pth+=2;
            char dirpart[VFS_PATH_MAX] = {0};
            char base[VFS_NAME_MAX] = {0};
            const char* slash = strrchr(pth, '/');
            if (slash) {
                size_t dlen = (size_t)(slash - pth);
                if (dlen >= sizeof(dirpart)) dlen = sizeof(dirpart)-1;
                memcpy(dirpart, pth, dlen);
                dirpart[dlen] = '\0';
                strncpy(base, slash+1, sizeof(base)-1);
            } else {
                strncpy(base, pth, sizeof(base)-1);
            }

            vfs_node_t* parent = ensure_dir(root, dirpart);
            if (!parent) parent = root;

            vfs_node_t* node = kmalloc(sizeof(vfs_node_t));
            if (!node) { off += 512 + ((size + 511) & ~511); continue; }
            memset(node, 0, sizeof(vfs_node_t));
            strncpy(node->name, base, sizeof(node->name)-1);
            node->flags = S_IFREG | (mode ? mode : 0644); // regular file with perms
            node->size = size;
            node->read = tar_read;
            node->open = tar_open;
            node->close = tar_close;
            node->parent = parent;

            tar_file_meta_t* meta = kmalloc(sizeof(tar_file_meta_t));
            if (!meta) { kfree(node); off += 512 + ((size + 511) & ~511); continue; }
            meta->data_off = off + 512;  // payload starts after header
            meta->size = size;
            node->data = meta; // store meta pointer; tar_read uses it

            vfs_node_add_child(parent, node);

            serial_write("[initrd] Added file: "); serial_write(node->name); serial_write("\n");
        } else if (hdr->typeflag == '5') {
            // Directory: ensure it exists in the hierarchy
            vfs_node_t* dir = ensure_dir(root, path);
            if (dir && (dir->flags & S_IFMT) == S_IFDIR && mode) {
                // apply permissions if provided
                dir->flags = (dir->flags & S_IFMT) | (mode ? mode : 0755);
            }
            (void)dir;
            serial_write("[initrd] Added directory: ");
            serial_write(path);
            serial_write("\n");
        }
        
        // Move to next entry (512-byte aligned)
        off += 512 + ((size + 511) & ~511);
    }
    
    vfs_set_root(root);
    serial_write("[initrd] Mounted as root filesystem\n");
    return 0;
}

static ssize_t tar_read_ram(vfs_node_t *ino, void *buf, size_t off, size_t len) {
    tar_file_meta_t *m = (tar_file_meta_t *)ino->priv;
    if (off >= m->size) return 0;
    if (off + len > m->size) len = m->size - off;
    memcpy(buf, g_initrd_img + m->data_off + off, len);
    return (ssize_t)len;
}

// Implement vfs_node_add_child function
void vfs_node_add_child(vfs_node_t* parent, vfs_node_t* child) {
    if (!parent || !child) return;
    child->next = parent->children;
    parent->children = child;
}

// Test function to read initial bytes of init.elf
void test_read_init_elf(void) {
    uint8_t hdr16[16];
    uint32_t bytes_read = 0;
    if (vfs_read_all("/bin/init.elf", hdr16, 16, &bytes_read) > 0) {
        serial_write("[initrd] First 16 bytes of /bin/init.elf: ");
        for (int i = 0; i < 16; i++) {
            serial_write_hex(hdr16[i]);
            serial_write(" ");
        }
        serial_write("\n");
    } else {
        serial_write("[initrd] Failed to read /bin/init.elf\n");
    }
}

// Mount a ustar initrd from a block device into the VFS as root (/).
int initrd_mount_from_block(const char* dev_name, uint32_t start_lba, uint32_t max_sectors, uint32_t max_bytes_cap) {
    serial_write("[initrd_mount_from_block] Mounting initrd\n");

    if (!dev_name || max_sectors == 0) {
        serial_write("[initrd_mount_from_block] Invalid parameters\n");
        return -1;
    }

    // Determine how many sectors to read within the byte cap
    uint32_t sector_size = 512; // our block layer/ATA uses 512-byte sectors
    uint32_t max_by_cap_sectors = max_bytes_cap / sector_size;
    uint32_t to_read = max_sectors;
    if (to_read > max_by_cap_sectors) to_read = max_by_cap_sectors;
    if (to_read == 0) {
        serial_write("[initrd_mount_from_block] to_read == 0 after cap\n");
        return -1;
    }

    // Allocate buffer for the initrd image
    uint32_t bytes = to_read * sector_size;
    uint8_t* buf = (uint8_t*)kmalloc(bytes);
    if (!buf) {
        serial_write("[initrd_mount_from_block] kmalloc failed\n");
        return -1;
    }

    // Read from block device
    extern int blk_read_byname(const char* name, uint32_t lba, uint32_t count, void* out);
    int rc = blk_read_byname(dev_name, start_lba, to_read, buf);
    if (rc != 0) {
        serial_write("[initrd_mount_from_block] blk_read_byname failed\n");
        kfree(buf);
        return rc ? rc : -1;
    }

    // Mount the initrd into the VFS
    if (mount_initrd(buf, bytes) < 0) {
        serial_write("[initrd_mount_from_block] Failed to mount initrd\n");
        kfree(buf);
        return -1;
    }

    serial_write("[initrd_mount_from_block] Initrd mounted successfully\n");
    // Keep buf allocated as long as filesystem is mounted (g_initrd_img points into it)
    return 0;
}
