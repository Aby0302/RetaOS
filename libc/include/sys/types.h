#ifndef _SYS_TYPES_H
#define _SYS_TYPES_H

#include <stddef.h>
#include <stdint.h>

// File descriptor and basic types
typedef int ssize_t;
// size_t is defined in <stddef.h>
typedef int pid_t;
typedef int mode_t;
typedef long off_t;
typedef int uid_t;
typedef int gid_t;

typedef unsigned int dev_t;
typedef unsigned int ino_t;
typedef unsigned int nlink_t;
typedef unsigned int blksize_t;
typedef unsigned int blkcnt_t;

typedef long time_t;
typedef long suseconds_t;
typedef unsigned long useconds_t;

// Directory types and APIs are declared in <dirent.h>

#endif /* _SYS_TYPES_H */
