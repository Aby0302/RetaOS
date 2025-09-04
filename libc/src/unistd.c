#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/syscall.h>

// System call wrapper
static long _syscall1(long n, long a1) {
    long ret;
    __asm__ volatile ("int $0x80" : "=a"(ret) : "a"(n), "b"(a1));
    return ret;
}

static long _syscall3(long n, long a1, long a2, long a3) {
    long ret;
    __asm__ volatile ("int $0x80" : "=a"(ret) : "a"(n), "b"(a1), "c"(a2), "d"(a3));
    return ret;
}

// System call wrappers
pid_t fork(void) {
    return _syscall1(SYS_FORK, 0);
}

pid_t getpid(void) {
    return _syscall1(SYS_GETPID, 0);
}

pid_t getppid(void) {
    return _syscall1(SYS_GETPPID, 0);
}

ssize_t write(int fd, const void *buf, size_t count) {
    return _syscall3(SYS_WRITE, fd, (long)buf, count);
}

ssize_t read(int fd, void *buf, size_t count) {
    return _syscall3(SYS_READ, fd, (long)buf, count);
}

int close(int fd) {
    return _syscall1(SYS_CLOSE, fd);
}

int execve(const char *path, char *const argv[], char *const envp[]) {
    return _syscall3(SYS_EXECVE, (long)path, (long)argv, (long)envp);
}

void _exit(int status) {
    _syscall1(SYS_EXIT, status);
    while (1); // Should never reach here
}

pid_t waitpid(pid_t pid, int *status, int options) {
    return _syscall3(SYS_WAITPID, pid, (long)status, options);
}

void *sbrk(intptr_t increment) {
    return (void*)_syscall1(SYS_SBRK, increment);
}

// Environment variables
extern char **environ;

// Get current working directory
#ifdef SYS_GETCWD
char *getcwd(char *buf, size_t size) {
    if (!buf) {
        // Allocate buffer if NULL is passed
        if (size == 0) size = 4096;
        buf = malloc(size);
        if (!buf) return NULL;
    }
    
    // Call getcwd system call
    long ret = _syscall3(SYS_GETCWD, (long)buf, size, 0);
    if (ret < 0) {
        if (buf) free(buf);
        return NULL;
    }
    
    return buf;
}
#endif

// Simple busy-wait sleep (fallback)
unsigned int sleep(unsigned int seconds) {
    volatile unsigned long loops = (unsigned long)seconds * 1000000UL;
    while (loops--) __asm__ __volatile__("pause");
    return 0;
}

// Minimal access(2) stub: allow execve to probe; return 0 always
int access(const char *pathname, int mode) {
    (void)pathname; (void)mode; return 0;
}

// Change directory
#ifdef SYS_CHDIR
int chdir(const char *path) {
    return _syscall1(SYS_CHDIR, (long)path);
}
#endif
