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
static void serial_putc(char ch){
  if(!serial_ready) return;
  while(!is_transmit_empty()) { }
  outb(COM1, (uint8_t)ch);
}
void serial_write(const char* s){
  if(!serial_ready) return;
  for(size_t i=0; s[i]; i++) serial_putc(s[i]);
}
void serial_write_hex(unsigned int v){
  const char* hex = "0123456789ABCDEF";
  serial_write("0x");
  for (int i = 7; i >= 0; --i) serial_putc(hex[(v >> (i*4)) & 0xF]);
}
void serial_write_dec(unsigned int v){
  char buf[16]; int i = 0;
  if (v == 0){ serial_putc('0'); return; }
  while (v && i < 15){ buf[i++] = '0' + (v % 10); v /= 10; }
  while (i--) serial_putc(buf[i]);
}
