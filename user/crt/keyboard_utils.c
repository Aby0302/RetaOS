#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <drivers/keyboard.h>

#define KERROR -1

void keyboard_clean_buffer() {
    // Clear the keyboard buffer
    while (keyboard_getchar_nonblock() != KERROR) {
        // Buffer temizleniyor
    }
    
    // Tuş basılmasını bekle
    while (keyboard_getchar_nonblock() == KERROR) {
        printf("Press any key to continue...");
        fflush(stdout);
        sleep(1);
    }
}

int main() {
    keyboard_clean_buffer();
    printf("Keyboard buffer successfully cleaned\n");
    return EXIT_SUCCESS;
}