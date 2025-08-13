#include "include/arch/x86/io.h"
#include "include/drivers/serial.h"
#include "include/drivers/keyboard.h"

#define KBD_DATA 0x60
#define BUF_SIZE 128

static volatile char ring[BUF_SIZE];
static volatile unsigned int head = 0, tail = 0;

static char scancode_to_ascii(uint8_t sc){
    static const char map[128] = {
        0,  27,'1','2','3','4','5','6','7','8','9','0','-','=', 8,  9,
       'q','w','e','r','t','y','u','i','o','p','[',']','\\', 0, 'a','s',
       'd','f','g','h','j','k','l',';','\'', '`', 0,'\\','z','x','c','v',
       'b','n','m',',','.','/', 0, '*', 0, ' ', // 0x39 = space
    };
    if (sc < sizeof(map)) return map[sc];
    return 0;
}

void keyboard_init(void){ head = tail = 0; }

static inline void push_char(char ch){ unsigned int n = (head + 1) % BUF_SIZE; if (n != tail){ ring[head] = ch; head = n; } }

int keyboard_getchar_nonblock(void){ if (head == tail) return -1; char ch = ring[tail]; tail = (tail + 1) % BUF_SIZE; return (int)ch; }

void keyboard_irq_handler(void){
    uint8_t sc = inb(KBD_DATA);
    if (sc & 0x80){ return; } // key release
    char ch = scancode_to_ascii(sc);
    if (ch){ push_char(ch); }
    // Optional: echo to serial
    // if (ch){ serial_write("[kbd] "); serial_write_hex(sc); serial_write(" '" ); serial_write((char[]){ch,0}); serial_write("'\n"); }
} 