#pragma once
#include <stdint.h>

void keyboard_init(void);
void keyboard_irq_handler(void);
int  keyboard_getchar_nonblock(void); // returns -1 if none 