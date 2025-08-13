#include "include/io.h"
#include <stddef.h>
#include <stdint.h>
#define COM1 0x3F8
static int serial_ready = 0;
void serial_init(void){
  outb(COM1 + 1, 0x00);
  outb(COM1 + 3, 0x80);
  outb(COM1 + 0, 0x01); /* 115200 baud */
  outb(COM1 + 1, 0x00);
  outb(COM1 + 3, 0x03);
  outb(COM1 + 2, 0xC7);
  outb(COM1 + 4, 0x0B);
  serial_ready = 1;
}
static int is_transmit_empty(){ return inb(COM1 + 5) & 0x20; }
void serial_write(const char* s){
  if(!serial_ready) return;
  for(size_t i=0; s[i]; i++){
    while(!is_transmit_empty()) { }
    outb(COM1, (uint8_t)s[i]);
  }
}
