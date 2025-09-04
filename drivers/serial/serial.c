#include "include/arch/x86/io.h"
#include <stddef.h>
#include <stdint.h>
#define COM1 0x3F8
static int serial_ready = 0;
void serial_init(void){
  // Disable all interrupts
  outb(COM1 + 1, 0x00);
  
  // Enable DLAB (set baud rate divisor)
  outb(COM1 + 3, 0x80);
  
  // Set divisor to 1 (lo byte) for 115200 baud
  outb(COM1 + 0, 0x01);
  
  // hi byte
  outb(COM1 + 1, 0x00);
  
  // 8 bits, no parity, one stop bit
  outb(COM1 + 3, 0x03);
  
  // Enable FIFO, clear them, with 14-byte threshold
  outb(COM1 + 2, 0xC7);
  
  // IRQs enabled, RTS/DSR set
  outb(COM1 + 4, 0x0B);
  
  // Test if serial is working by checking if we can read back the scratch register
  outb(COM1 + 7, 0xAE);
  if(inb(COM1 + 7) != 0xAE) {
    // If we can't read back, serial port might not be available
    return;
  }
  
  // Send a test character
  outb(COM1, 'S');
  outb(COM1, 'E');
  outb(COM1, 'R');
  outb(COM1, 'I');
  outb(COM1, 'A');
  outb(COM1, 'L');
  outb(COM1, '\r');
  outb(COM1, '\n');
  
  serial_ready = 1;
}
static int is_transmit_empty(){ return inb(COM1 + 5) & 0x20; }
static int serial_data_ready(){ return inb(COM1 + 5) & 0x01; }
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

// Non-blocking input read; returns -1 if no byte available
int serial_getchar_nonblock(void){
  if(!serial_ready) return -1;
  if (!serial_data_ready()) return -1;
  return (int)(uint8_t)inb(COM1);
}
