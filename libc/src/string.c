#include <string.h>

size_t strlen(const char *s) {
    const char *p = s;
    while (*p) p++;
    return p - s;
}

char *strcpy(char *dest, const char *src) {
    char *ret = dest;
    while ((*dest++ = *src++));
    return ret;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && *s1 == *s2) { s1++; s2++; }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    while (n-- && *s1 && (*s1 == *s2)) { s1++; s2++; }
    if ((size_t)(n+1) == 0) return 0; // compared n chars
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

char *strncat(char *dest, const char *src, size_t n) {
    char *d = dest + strlen(dest);
    while (n-- && *src) *d++ = *src++;
    *d = '\0';
    return dest;
}

char *strcat(char *dest, const char *src) {
    char *d = dest + strlen(dest);
    while ((*d++ = *src++));
    return dest;
}

// Very simple strtok (not re-entrant). Delims treated as set of bytes.
char *strtok(char *str, const char *delim) {
    static char *save;
    if (str) save = str; else if (!save) return NULL;
    // skip leading delims
    char *s = save; const char *d; int isdelim;
    while (*s) {
        isdelim = 0; for (d = delim; *d; ++d) if (*s == *d) { isdelim = 1; break; }
        if (!isdelim) break; ++s;
    }
    if (!*s) { save = NULL; return NULL; }
    char *tok = s;
    while (*s) {
        isdelim = 0; for (d = delim; *d; ++d) if (*s == *d) { isdelim = 1; break; }
        if (isdelim) { *s++ = '\0'; break; }
        ++s;
    }
    save = *s ? s : NULL;
    return tok;
}

void *memset(void *s, int c, size_t n) {
    unsigned char *p = s;
    while (n--) *p++ = (unsigned char)c;
    return s;
}

void *memcpy(void *dest, const void *src, size_t n) {
    char *d = dest;
    const char *s = src;
    while (n--) *d++ = *s++;
    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const unsigned char *p1 = s1, *p2 = s2;
    while (n-- > 0) {
        if (*p1 != *p2) return *p1 - *p2;
        p1++; p2++;
    }
    return 0;
}
