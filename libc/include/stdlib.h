#ifndef _STDLIB_H
#define _STDLIB_H

#include <stddef.h>

// Standard exit status codes
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

// Memory allocation
void *malloc(size_t size);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);

// Process control
void _Exit(int status) __attribute__((noreturn));
void exit(int status) __attribute__((noreturn));
int atexit(void (*func)(void));
int system(const char *command);

// Environment
char *getenv(const char *name);
int setenv(const char *name, const char *value, int overwrite);
int unsetenv(const char *name);
int putenv(char *string);

// String conversions
double atof(const char *nptr);
int atoi(const char *nptr);
long atol(const char *nptr);
long long atoll(const char *nptr);
long strtol(const char *nptr, char **endptr, int base);
unsigned long strtoul(const char *nptr, char **endptr, int base);
long long strtoll(const char *nptr, char **endptr, int base);
unsigned long long strtoull(const char *nptr, char **endptr, int base);

// Pseudo-random sequence generation
int rand(void);
void srand(unsigned int seed);

// Searching and sorting
void *bsearch(const void *key, const void *base,
              size_t nmemb, size_t size,
              int (*compar)(const void *, const void *));
void qsort(void *base, size_t nmemb, size_t size,
           int (*compar)(const void *, const void *));

// Integer arithmetic
int abs(int j);
long int labs(long int j);
long long int llabs(long long int j);

typedef struct {
    int quot; // Quotient
    int rem;  // Remainder
} div_t;

div_t div(int numer, int denom);

// Multibyte/wide character conversion
int mblen(const char *s, size_t n);
int mbtowc(wchar_t *pwc, const char *s, size_t n);
int wctomb(char *s, wchar_t wc);

// Multibyte string functions
size_t mbstowcs(wchar_t *dest, const char *src, size_t n);
size_t wcstombs(char *dest, const wchar_t *src, size_t n);

// Non-local jumps
#include <setjmp.h>

// Communication with the environment
char *getenv(const char *name);
int system(const char *command);

// Searching and sorting utilities
void *bsearch(const void *key, const void *base,
              size_t nmemb, size_t size,
              int (*compar)(const void *, const void *));
void qsort(void *base, size_t nmemb, size_t size,
           int (*compar)(const void *, const void *));

// Absolute value
int abs(int j);
long int labs(long int j);
long long int llabs(long long int j);

// Integer division
div_t div(int numer, int denom);
ldiv_t ldiv(long numer, long denom);
lldiv_t lldiv(long long numer, long long denom);

// Type-generic math macro
#ifndef __cplusplus
#define abs(j) ((j) < 0 ? -(j) : (j))
#endif

#endif /* _STDLIB_H */
