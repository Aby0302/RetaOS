#ifndef _STDIO_H
#define _STDIO_H

#include <stddef.h>
#include <stdarg.h>

// File stream type (simplified)
typedef struct {
    int fd;           // File descriptor
    int flags;        // File status flags
    off_t pos;        // Current position
    unsigned char *buf;// Buffer
    size_t bufsize;   // Buffer size
} FILE;

// Standard streams
extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

// File operations
FILE *fopen(const char *pathname, const char *mode);
int fclose(FILE *stream);
int fflush(FILE *stream);

// Character I/O
int fputc(int c, FILE *stream);
int putc(int c, FILE *stream);
int putchar(int c);
int fgetc(FILE *stream);
int getc(FILE *stream);
int getchar(void);
int ungetc(int c, FILE *stream);

// String I/O
char *fgets(char *s, int size, FILE *stream);
char *gets(char *s);  // Unsafe, included for compatibility
int fputs(const char *s, FILE *stream);
int puts(const char *s);

// Formatted I/O
int printf(const char *format, ...);
int fprintf(FILE *stream, const char *format, ...);
int sprintf(char *str, const char *format, ...);
int snprintf(char *str, size_t size, const char *format, ...);
int vprintf(const char *format, va_list ap);
int vfprintf(FILE *stream, const char *format, va_list ap);
int vsprintf(char *str, const char *format, va_list ap);
int vsnprintf(char *str, size_t size, const char *format, va_list ap);
int scanf(const char *format, ...);
int fscanf(FILE *stream, const char *format, ...);
int sscanf(const char *str, const char *format, ...);

// File positioning
int fseek(FILE *stream, long offset, int whence);
long ftell(FILE *stream);
void rewind(FILE *stream);
int fgetpos(FILE *stream, fpos_t *pos);
int fsetpos(FILE *stream, const fpos_t *pos);

// Error handling
void perror(const char *s);
int ferror(FILE *stream);
int feof(FILE *stream);
void clearerr(FILE *stream);

// Temporary files
FILE *tmpfile(void);
char *tmpnam(char *s);

// File operations using file descriptors
FILE *fdopen(int fd, const char *mode);
int fileno(FILE *stream);

// Buffer control
void setbuf(FILE *stream, char *buf);
int setvbuf(FILE *stream, char *buf, int mode, size_t size);

// File positioning macros
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

// End-of-file indicator
#define EOF (-1)

// Buffer modes for setvbuf
#define _IOFBF 0  // Fully buffered
#define _IOLBF 1  // Line buffered
#define _IONBF 2  // No buffering

// Standard file pointers
#define stdin  ((FILE*)0)
#define stdout ((FILE*)1)
#define stderr ((FILE*)2)

// Type for fpos_t (file position)
typedef long fpos_t;

#endif /* _STDIO_H */
