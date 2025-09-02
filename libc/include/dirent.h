#ifndef _DIRENT_H
#define _DIRENT_H

#include <sys/types.h>

// Directory stream type
typedef struct {
    int fd;  // File descriptor for the directory
    // Other implementation-specific members
} DIR;

// Directory entry structure
struct dirent {
    ino_t          d_ino;       // Inode number
    off_t          d_off;       // Offset to next dirent
    unsigned short d_reclen;    // Length of this record
    unsigned char  d_type;      // Type of file
    char           d_name[256]; // Filename (null-terminated)
};

// File types for d_type
#define DT_UNKNOWN   0   // Unknown type
#define DT_FIFO      1   // Named pipe (FIFO)
#define DT_CHR       2   // Character device
#define DT_DIR       4   // Directory
#define DT_BLK       6   // Block device
#define DT_REG       8   // Regular file
#define DT_LNK      10   // Symbolic link
#define DT_SOCK     12   // UNIX domain socket
#define DT_WHT      14   // Whiteout (BSD)

// Directory operations
DIR *opendir(const char *name);
struct dirent *readdir(DIR *dirp);
int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result);
int closedir(DIR *dirp);
void rewinddir(DIR *dirp);
void seekdir(DIR *dirp, long loc);
long telldir(DIR *dirp);

// Additional POSIX functions
int dirfd(DIR *dirp);
int scandir(const char *dirp, struct dirent ***namelist,
           int (*filter)(const struct dirent *),
           int (*compar)(const struct dirent **, const struct dirent **));
int alphasort(const struct dirent **a, const struct dirent **b);

// Additional functions
int getdents(unsigned int fd, struct dirent *dirp, unsigned int count);

#ifndef _POSIX_SOURCE
#define MAXNAMLEN 255
#endif

#endif /* _DIRENT_H */
