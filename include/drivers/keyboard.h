#pragma once
#include <stdint.h>

void keyboard_init(void);
void keyboard_irq_handler(void);
int  keyboard_getchar_nonblock(void); // returns -1 if none 

// Special keycodes (returned by keyboard_getkey_nonblock)
enum {
    KBD_KEY_BASE = 0x100,
    KBD_LEFT  = 0x101,
    KBD_RIGHT = 0x102,
    KBD_UP    = 0x103,
    KBD_DOWN  = 0x104,
    KBD_HOME  = 0x105,
    KBD_END   = 0x106,
    KBD_DEL   = 0x107,
};

// Returns ASCII for printable keys and special codes above (>= KBD_KEY_BASE) for navigation keys.
// Returns -1 if no key pending.
int keyboard_getkey_nonblock(void);