#include "include/serial.h"
#include "include/io.h"
#include "include/isr.h"

static volatile unsigned int ticks = 0;

static void timer_tick(void){
  ticks++;
  if ((ticks % 100) == 0){
    serial_write("[tick] "); serial_write_dec(ticks); serial_write("\n");
  }
}

static void keyboard_irq(void){
  uint8_t sc = inb(0x60);
  serial_write("[kbd] scancode="); serial_write_hex(sc); serial_write("\n");
}

static void pit_init_100hz(void){
  uint32_t freq = 100; uint32_t div = 1193180 / freq;
  outb(0x43, 0x36);
  outb(0x40, div & 0xFF);
  outb(0x40, (div >> 8) & 0xFF);
}

static void pic_unmask_timer_keyboard(void){
  // Unmask IRQ0 (timer) and IRQ1 (keyboard); others masked
  outb(0x21, 0xFC); // 11111100b -> IRQ0,1 enabled
  outb(0xA1, 0xFF);
}

void isr14_handler(void){ serial_write("[RetaOS] Page Fault!\n"); }

// Initialization hook to be called after idt_init()
void irq_init_basic(void){
  irq_install_handler(0, timer_tick);
  irq_install_handler(1, keyboard_irq);
  pit_init_100hz();
  pic_unmask_timer_keyboard();
}
