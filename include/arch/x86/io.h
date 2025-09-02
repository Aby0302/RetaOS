#pragma once
#include <stdint.h>
static inline void outb(uint16_t port, uint8_t val){
  __asm__ __volatile__ ("outb %0, %1" : : "a"(val), "Nd"(port));
}
static inline uint8_t inb(uint16_t port){
  uint8_t ret;
  __asm__ __volatile__ ("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}
static inline void io_wait(void){ outb(0x80, 0); }

static inline void outw(uint16_t port, uint16_t val){
  __asm__ __volatile__("outw %0, %1" : : "a"(val), "Nd"(port));
}
static inline uint16_t inw(uint16_t port){
  uint16_t ret;
  __asm__ __volatile__("inw %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}
static inline void insw(uint16_t port, void* addr, uint32_t count){
  __asm__ __volatile__("rep insw" : "+D"(addr), "+c"(count) : "d"(port) : "memory");
}
