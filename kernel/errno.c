#include <errno.h>

// Provide a kernel-local errno storage and accessor, compatible with our libc's errno.h
static int kernel_errno = 0;

int* __errno_location(void) {
    return &kernel_errno;
}