#ifndef _WCHAR_H
#define _WCHAR_H

#include <stddef.h>
#include <stdint.h>

// Provide minimal wide-char related types for our freestanding libc
typedef unsigned int wint_t;    // wide int type for wide char functions

typedef struct {
    int __dummy; // placeholder; real mbstate_t not implemented
} mbstate_t;

#endif // _WCHAR_H