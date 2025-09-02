#include <stddef.h>

int strncmp(const char* s1, const char* s2, size_t n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    
    if (n == 0) {
        return 0;
    } else {
        return (*(unsigned char*)s1 - *(unsigned char*)s2);
    }
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

char* strncpy(char* dest, const char* src, size_t n) {
    char* ret = dest;
    while (n && (*dest = *src)) {
        dest++;
        src++;
        n--;
    }
    while (n--) {
        *dest++ = '\0';
    }
    return ret;
}

char* strcpy(char* dest, const char* src) {
    char* ret = dest;
    while ((*dest = *src)) {
        dest++;
        src++;
    }
    return ret;
}

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) {
        len++;
    }
    return len;
}

char* strrchr(const char* str, int c) {
    char* last = NULL;
    while (*str) {
        if (*str == c) {
            last = (char*)str;
        }
        str++;
    }
    return last;
}

char* strpbrk(const char* str, const char* accept) {
    while (*str) {
        const char* a = accept;
        while (*a) {
            if (*str == *a) {
                return (char*)str;
            }
            a++;
        }
        str++;
    }
    return NULL;
}

size_t strspn(const char* str, const char* accept) {
    size_t count = 0;
    while (str[count]) {
        const char* a = accept;
        int found = 0;
        while (*a) {
            if (str[count] == *a) {
                found = 1;
                break;
            }
            a++;
        }
        if (!found) {
            break;
        }
        count++;
    }
    return count;
}

int tolower(int c) {
    if (c >= 'A' && c <= 'Z') {
        return c + ('a' - 'A');
    }
    return c;
}

int toupper(int c) {
    if (c >= 'a' && c <= 'z') {
        return c - ('a' - 'A');
    }
    return c;
}
