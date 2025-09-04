#include "sound.h"
#include <kernel/console.h> // Correct header for kprintf

void sound_init(void) {
    kprintf("Sound driver initialized.\n");
}
