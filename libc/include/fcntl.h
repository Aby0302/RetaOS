#ifndef _FCNTL_H
#define _FCNTL_H

#include <sys/types.h>

// File access modes
#define O_RDONLY  0x0000
#define O_WRONLY  0x0001
#define O_RDWR    0x0002
#define O_ACCMODE 0x0003

// File creation flags
#define O_CREAT   0x0040
#define O_EXCL    0x0080
#define O_NOCTTY  0x0100
#define O_TRUNC   0x0200
#define O_APPEND  0x0400
#define O_NONBLOCK 0x0800

// File status flags
#define O_DSYNC   0x1000
#define O_ASYNC   0x2000
#define O_DIRECT  0x4000

// File control commands
#define F_DUPFD   0
#define F_GETFD   1
#define F_SETFD   2
#define F_GETFL   3
#define F_SETFL   4

// File locking
struct flock {
    short l_type;
    short l_whence;
    off_t l_start;
    off_t l_len;
    pid_t l_pid;
};

// Lock types
#define F_RDLCK   0
#define F_WRLCK   1
#define F_UNLCK   2

// Function declarations
int open(const char *path, int flags, ...);
int creat(const char *path, mode_t mode);
int fcntl(int fd, int cmd, ...);

#endif /* _FCNTL_H */
