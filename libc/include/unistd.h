#ifndef _UNISTD_H
#define _UNISTD_H

#include <sys/types.h>
#include <stdint.h>  // For intptr_t
#include <stddef.h>  // For size_t and ssize_t
#include <dirent.h>  // DIR and struct dirent declarations

// Standard file descriptors
#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

// File access modes for open()
#define O_RDONLY    0x0000  // Open for reading only
#define O_WRONLY    0x0001  // Open for writing only
#define O_RDWR      0x0002  // Open for reading and writing
#define O_ACCMODE   0x0003  // Mask for access modes

// File creation flags for open()
#define O_CREAT     0x0040  // Create file if it doesn't exist
#define O_EXCL      0x0080  // Fail if file already exists
#define O_TRUNC     0x0200  // Truncate file on open
#define O_APPEND    0x0400  // Append to file

// access(2) mode flags
#ifndef F_OK
#define F_OK 0
#define X_OK 1
#define W_OK 2
#define R_OK 4
#endif

// Process control
pid_t fork(void);
int execve(const char *path, char *const argv[], char *const envp[]);
pid_t getpid(void);
pid_t getppid(void);
pid_t waitpid(pid_t pid, int *status, int options);
void _exit(int status) __attribute__((noreturn));

// File operations
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
int open(const char *pathname, int flags, ...);
int close(int fd);
int access(const char *pathname, int mode);
off_t lseek(int fd, off_t offset, int whence);
int unlink(const char *pathname);
int rmdir(const char *pathname);
int chdir(const char *path);
char *getcwd(char *buf, size_t size);

// Directory operations are declared in <dirent.h>

// Memory management
void *sbrk(intptr_t increment);

// System calls
#include <sys/syscall.h>

// Standard C library functions
unsigned int sleep(unsigned int seconds);
int usleep(useconds_t usec);

// Environment variables
extern char **environ;
char *getenv(const char *name);
int setenv(const char *name, const char *value, int overwrite);
int unsetenv(const char *name);

// Process groups
pid_t getpgrp(void);
int setpgid(pid_t pid, pid_t pgid);
pid_t setsid(void);
pid_t getsid(pid_t pid);

// Terminal I/O
int isatty(int fd);
char *ttyname(int fd);

// System configuration
long sysconf(int name);

// Symbolic constants for sysconf()
#define _SC_PAGESIZE 0
#define _SC_CLK_TCK  1

#endif /* _UNISTD_H */
