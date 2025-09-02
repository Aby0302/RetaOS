#ifndef _CTYPE_H
#define _CTYPE_H

#include <stddef.h>
#include <wchar.h> // for wint_t

// Character classification functions
int isalnum(int c);
int isalpha(int c);
int iscntrl(int c);
int isdigit(int c);
int isgraph(int c);
int islower(int c);
int isprint(int c);
int ispunct(int c);
int isspace(int c);
int isupper(int c);
int isxdigit(int c);

// Character case mapping functions
// int tolower(int c);
// int toupper(int c);

// ASCII character tests (internal use)
#define _U  0x01    // Upper case
#define _L  0x02    // Lower case
#define _N  0x04    // Numeral (digit)
#define _S  0x08    // Spacing character
#define _P  0x10    // Punctuation
#define _C  0x20    // Control character
#define _X  0x40    // Hexadecimal digit
#define _B  0x80    // Blank (space or tab)

// Character type table (initialized in ctype.c)
extern const unsigned char _ctype_[256];

// Implementation of character classification functions
#define isalnum(c)  (_ctype_[(unsigned char)(c)] & (_U|_L|_N))
#define isalpha(c)  (_ctype_[(unsigned char)(c)] & (_U|_L))
#define iscntrl(c)  (_ctype_[(unsigned char)(c)] & _C)
#define isdigit(c)  ((c) >= '0' && (c) <= '9')
#define isgraph(c)  ((c) > ' ' && (c) <= '~')
#define islower(c)  ((c) >= 'a' && (c) <= 'z')
#define isprint(c)  ((c) >= ' ' && (c) <= '~')
#define ispunct(c)  (_ctype_[(unsigned char)(c)] & _P)
#define isspace(c)  (_ctype_[(unsigned char)(c)] & _S)
#define isupper(c)  ((c) >= 'A' && (c) <= 'Z')
#define isxdigit(c) (isdigit(c) || ((c) >= 'a' && (c) <= 'f') || ((c) >= 'A' && (c) <= 'F'))

// Implementation of case mapping functions
static inline int tolower(int c) {
    return isupper(c) ? (c) - 'A' + 'a' : c;
}

static inline int toupper(int c) {
    return islower(c) ? (c) - 'a' + 'A' : c;
}

// Wide character functions
int iswalnum(wint_t wc);
int iswalpha(wint_t wc);
int iswcntrl(wint_t wc);
int iswdigit(wint_t wc);
int iswgraph(wint_t wc);
int iswlower(wint_t wc);
int iswprint(wint_t wc);
int iswpunct(wint_t wc);
int iswspace(wint_t wc);
int iswupper(wint_t wc);
int iswxdigit(wint_t wc);

// Wide character case mapping
wint_t towlower(wint_t wc);
wint_t towupper(wint_t wc);

// Character class functions
int isblank(int c);
int isascii(int c);
int toascii(int c);

#endif /* _CTYPE_H */
