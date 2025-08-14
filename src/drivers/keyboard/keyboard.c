#include "include/arch/x86/io.h"
#include "include/drivers/serial.h"
#include "include/drivers/keyboard.h"

#define KBD_DATA 0x60
#define BUF_SIZE 128

static volatile uint16_t ring[BUF_SIZE];
static volatile unsigned int head = 0, tail = 0;

// Modifier state
static volatile int shift_l = 0, shift_r = 0, caps = 0;
static volatile int e0_pending = 0;

static char scancode_to_ascii(uint8_t sc){
    static const char map[128] = {
        0,  27,'1','2','3','4','5','6','7','8','9','0','-','=', 8,  9,
       'q','w','e','r','t','y','u','i','o','p','[',']','\\', 0, 'a','s',
       'd','f','g','h','j','k','l',';','\'', '`', 0,'\\','z','x','c','v',
        'b','n','m',',','.','/', 0, '*', 0, ' ',
    };
    static const char map_shift[128] = {
        0,  27,'!','@','#','$','%','^','&','*','(',')','_','+', 8,  9,
       'Q','W','E','R','T','Y','U','I','O','P','{','}','|', 0, 'A','S',
       'D','F','G','H','J','K','L',':','\"','~', 0,'|','Z','X','C','V',
        'B','N','M','<','>','?', 0, '*', 0, ' ',
    };
    if (sc == 0x1C) return '\n';       // Enter
    if (sc == 0x0E) return 8;           // Backspace
    if (sc < sizeof(map)){
        int is_letter = ( (sc>=0x10 && sc<=0x19) || (sc>=0x1E && sc<=0x26) || (sc>=0x2C && sc<=0x32) );
        int sh = (shift_l || shift_r);
        if (is_letter) {
            // For letters, CapsLock XOR Shift
            if (caps ^ sh) return map_shift[sc];
            else return map[sc];
        } else {
            // For non-letters, only Shift matters
            return sh ? map_shift[sc] : map[sc];
        }
    }
    return 0;
}

void keyboard_init(void){ head = tail = 0; shift_l = shift_r = caps = 0; e0_pending = 0; }

static inline void push_key(uint16_t key){ unsigned int n = (head + 1) % BUF_SIZE; if (n != tail){ ring[head] = key; head = n; } }

int keyboard_getkey_nonblock(void){ if (head == tail) return -1; uint16_t k = ring[tail]; tail = (tail + 1) % BUF_SIZE; return (int)k; }

int keyboard_getchar_nonblock(void){
    int k = keyboard_getkey_nonblock();
    if (k < 0) return -1;
    if (k < 256) return k; // ASCII or control like 8/\n
    return -1;
}

void keyboard_irq_handler(void){
    uint8_t sc = inb(KBD_DATA);
    if (sc == 0xE0){ e0_pending = 1; return; }
    int release = sc & 0x80; uint8_t code = sc & 0x7F;

    if (e0_pending){
        e0_pending = 0;
        if (release){ return; }
        // Extended keys
        switch (code){
            case 0x4B: push_key(KBD_LEFT); return;   // Left
            case 0x4D: push_key(KBD_RIGHT); return;  // Right
            case 0x48: push_key(KBD_UP); return;     // Up
            case 0x50: push_key(KBD_DOWN); return;   // Down
            case 0x47: push_key(KBD_HOME); return;   // Home
            case 0x4F: push_key(KBD_END); return;    // End
            case 0x53: push_key(KBD_DEL); return;    // Delete
            default: return;
        }
    }

    // Non-extended keys
    if (release){
        if (code == 0x2A) shift_l = 0; // LShift
        else if (code == 0x36) shift_r = 0; // RShift
        return;
    }

    // Presses
    if (code == 0x2A) { shift_l = 1; return; }
    if (code == 0x36) { shift_r = 1; return; }
    if (code == 0x3A) { caps ^= 1; return; } // CapsLock toggle

    char ch = scancode_to_ascii(code);
    if (ch){ push_key((uint16_t)(uint8_t)ch); }
}
// Optional: echo to serial
// In IRQ handler above, you can add serial debug prints if needed.