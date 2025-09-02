#ifndef _SYS_STAT_H
#define _SYS_STAT_H

#include <sys/types.h>

// File type and mode bits (st_mode)
#define S_IFMT   0170000  // Bit mask for the file type bit field
#define S_IFSOCK 0140000  // Socket
#define S_IFLNK  0120000  // Symbolic link
#define S_IFREG  0100000  // Regular file
#define S_IFBLK  0060000  // Block device
#define S_IFDIR  0040000  // Directory
#define S_IFCHR  0020000  // Character device
#define S_IFIFO  0010000  // FIFO

// File mode bits
#define S_ISUID  0004000  // Set UID bit
#define S_ISGID  0002000  // Set GID bit
#define S_ISVTX  0001000  // Sticky bit

// Permission bits
#define S_IRWXU 00700    // Owner: rwx
#define S_IRUSR 00400    // Owner: read
#define S_IWUSR 00200    // Owner: write
#define S_IXUSR 00100    // Owner: execute

#define S_IRWXG 00070    // Group: rwx
#define S_IRGRP 00040    // Group: read
#define S_IWGRP 00020    // Group: write
#define S_IXGRP 00010    // Group: execute

#define S_IRWXO 00007    // Others: rwx
#define S_IROTH 00004    // Others: read
#define S_IWOTH 00002    // Others: write
#define S_IXOTH 00001    // Others: execute

// Macros to test file types
#define S_ISREG(m)  (((m) & S_IFMT) == S_IFREG)  // Regular file
#define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)  // Directory
#define S_ISCHR(m)  (((m) & S_IFMT) == S_IFCHR)  // Character device
#define S_ISBLK(m)  (((m) & S_IFMT) == S_IFBLK)  // Block device
#define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)  // FIFO/pipe
#define S_ISLNK(m)  (((m) & S_IFMT) == S_IFLNK)  // Symbolic link
#define S_ISSOCK(m) (((m) & S_IFMT) == S_IFSOCK) // Socket

// File status structure
struct stat {
    dev_t     st_dev;         // ID of device containing file
    ino_t     st_ino;         // Inode number
    mode_t    st_mode;        // File type and mode
    nlink_t   st_nlink;       // Number of hard links
    uid_t     st_uid;         // User ID of owner
    gid_t     st_gid;         // Group ID of owner
    dev_t     st_rdev;        // Device ID (if special file)
    off_t     st_size;        // Total size, in bytes
    blksize_t st_blksize;     // Block size for filesystem I/O
    blkcnt_t  st_blocks;      // Number of 512B blocks allocated
    
    // Timespec structs for timestamps
    time_t    st_atime;       // Time of last access
    time_t    st_mtime;       // Time of last modification
    time_t    st_ctime;       // Time of last status change
};

// Function declarations
int chmod(const char *path, mode_t mode);
int fchmod(int fd, mode_t mode);
int fstat(int fd, struct stat *buf);
int lstat(const char *path, struct stat *buf);
int mkdir(const char *path, mode_t mode);
int mkfifo(const char *path, mode_t mode);
int mknod(const char *path, mode_t mode, dev_t dev);
int stat(const char *path, struct stat *buf);
mode_t umask(mode_t mask);

// For backward compatibility
#define S_IREAD   S_IRUSR
#define S_IWRITE  S_IWUSR
#define S_IEXEC   S_IXUSR

#endif /* _SYS_STAT_H */
