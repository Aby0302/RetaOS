#include "include/kernel/initrd.h"
#include "include/kernel/vfs.h"
#include "include/kernel/block.h"
#include "include/drivers/serial.h"
#include "include/memory/heap.h"
#include <stddef.h>

// Minimal ustar definitions
typedef struct __attribute__((packed)) ustar_hdr {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag; // '0' file, '5' dir
    char linkname[100];
    char magic[6]; // "ustar\0"
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char pad[12];
} ustar_hdr_t;

static uint32_t oct2u(const char* s, int n){
    uint32_t v=0; int i=0;
    while (i<n && s[i]){
        char c=s[i++];
        if (c<'0' || c>'7') break;
        v = (v<<3) + (uint32_t)(c - '0');
    }
    return v;
}

static int is_endblock(const uint8_t* b){
    for (int i=0;i<512;i++) {
        if (b[i]!=0) return 0;
    }
    return 1;
}

static vfs_node_t* make_dir_node(const char* name, vfs_node_t* parent){
    vfs_node_t* n = (vfs_node_t*)kmalloc(sizeof(vfs_node_t));
    // Copy name to node's name field
    int i = 0;
    while (name[i] && i < VFS_NAME_MAX - 1) {
        n->name[i] = name[i];
        i++;
    }
    n->name[i] = '\0';
    
    n->flags = FT_DIR;  // Directory flag
    n->size = 0;
    n->inode = 0;
    n->impl = 0;
    
    // Initialize function pointers
    n->read = NULL;
    n->write = NULL;
    n->open = NULL;
    n->close = NULL;
    n->readdir = NULL;
    n->finddir = NULL;
    
    n->priv = NULL;
    n->parent = parent;
    n->children = NULL;  // First child
    n->next = NULL;     // Next sibling
    
    return n;
}

static vfs_node_t* make_file_node(const char* name, const uint8_t* data, uint32_t size, vfs_node_t* parent){
    vfs_node_t* n = (vfs_node_t*)kmalloc(sizeof(vfs_node_t));
    // Copy name to node's name field
    int i = 0;
    while (name[i] && i < VFS_NAME_MAX - 1) {
        n->name[i] = name[i];
        i++;
    }
    n->name[i] = '\0';
    
    n->flags = FT_FILE;  // File flag
    n->size = size;
    n->inode = 0;
    n->impl = 0;
    
    // Initialize function pointers
    n->read = NULL;
    n->write = NULL;
    n->open = NULL;
    n->close = NULL;
    n->readdir = NULL;
    n->finddir = NULL;
    
    // Store file data in priv field
    n->priv = (void*)data;
    n->parent = parent;
    n->children = NULL;  // No children for files
    n->next = NULL;     // Next sibling
    
    return n;
}

static void dir_add_child(vfs_node_t* dir, vfs_node_t* child){
    // Add to the beginning of the children list
    child->next = dir->children;
    dir->children = child;
}

static int path_next(const char* p, int* comp_len){
    int i=0; while (p[i] && p[i]!='/') i++; *comp_len=i; return p[i]=='/' ? (i+1) : i; // returns step to next start
}

static vfs_node_t* ensure_path(vfs_node_t* root, char* path){
    // path without leading '/'; creates directories as needed
    vfs_node_t* cur = root;
    char* p = path;
    while (*p){
        int len=0; int step = path_next(p, &len);
        char saved = p[len]; p[len]='\0';
        // lookup existing child
        vfs_node_t* found = NULL;
        vfs_node_t* child = cur->children;
        while (child) {
            // Check if this is a directory with matching name
            int eq = 1;
            int ai = 0;
            while (child->name[ai] && p[ai] && child->name[ai] == p[ai]) ai++;
            if (child->name[ai] || p[ai]) eq = 0;
            
            if ((child->flags & FT_DIR) && eq) {
                found = child;
                break;
            }
            child = child->next;
        }
        if (!found){
            // Create new directory node with the current path component
            found = make_dir_node(p, cur);
            dir_add_child(cur, found);
        }
        p[len]=saved;
        if (p[step-1]=='\0') return found; // last component was directory marker (shouldn't happen here)
        p += step;
        cur = found;
    }
    return cur;
}

int initrd_mount_from_block(const char* dev_name, uint32_t start_lba, uint32_t max_sectors, uint32_t max_bytes_cap) {
    block_dev_t* dev = blk_find(dev_name);
    if (!dev) { 
        serial_write("[initrd] device not found\n"); 
        return -1; 
    }
    
    if (dev->sector_size != 512) { 
        serial_write("[initrd] sector != 512 not supported\n"); 
        return -2; 
    }
    
    if (max_sectors == 0) return -3;
    
    uint32_t bytes = max_sectors * dev->sector_size;
    if (bytes > max_bytes_cap) bytes = max_bytes_cap;

    uint8_t* img = (uint8_t*)kmalloc(bytes);
    if (!img) { 
        serial_write("[initrd] alloc failed\n"); 
        return -4; 
    }
    
    uint32_t sectors_to_read = (bytes + 511) / 512;
    if (blk_read(dev, start_lba, sectors_to_read, img) != 0) { 
        serial_write("[initrd] read error\n"); 
        kfree(img);
        return -5; 
    }

    // Build VFS tree
    vfs_node_t* root = make_dir_node("", NULL);

    uint32_t off = 0;
    while (off + 512 <= bytes){
        const uint8_t* block = img + off;
        if (is_endblock(block)) break;
        const ustar_hdr_t* h = (const ustar_hdr_t*)block;
        // compute full name
        char full[300]; int fi=0;
        for (int i=0;i<155 && h->prefix[i];i++) full[fi++]=h->prefix[i];
        if (fi>0){ full[fi++]='/'; }
        for (int i=0;i<100 && h->name[i];i++) full[fi++]=h->name[i];
        full[fi]='\0';
        uint32_t fsize = oct2u(h->size, sizeof(h->size));
        char type = h->typeflag ? h->typeflag : '0';

        // split into dir + base
        // find last '/'
        int last=-1; for (int i=0;i<fi;i++){ if (full[i]=='/') last=i; }
        vfs_node_t* parent = root;
        if (last >= 0){
            full[last] = '\0';
            parent = ensure_path(root, full);
            full[last] = '/';
        }
        const char* base = (last>=0) ? (full + last + 1) : full;
        // copy base name
        int blen=0; while (base[blen]) blen++;
        char* bcopy = (char*)kmalloc(blen+1); for (int i=0;i<blen;i++) bcopy[i]=base[i]; bcopy[blen]='\0';

        if (type == '5'){
            vfs_node_t* dn = make_dir_node(bcopy, parent);
            dir_add_child(parent, dn);
        } else {
            const uint8_t* fdata = img + off + 512;
            vfs_node_t* fn = make_file_node(bcopy, fdata, fsize, parent);
            dir_add_child(parent, fn);
        }
        // Skip to next header
        off += 512;  // Skip header
        if (type != '5') {  // Skip file data for non-directory entries
            uint32_t blocks = (fsize + 511) / 512;
            off += blocks * 512;
        }
    }

    vfs_set_root(root);
    serial_write("[initrd] mounted as root VFS\n");
    kfree(img); // Free the image buffer after mounting
    return 0;
}
