#include "include/drivers/serial.h"
#include "include/arch/x86/io.h"
#include "include/arch/x86/isr.h"
#include <stdint.h>

// forward decls from drivers
void keyboard_irq_handler(void);

static volatile unsigned int ticks = 0;

static void vga_panic_screen(const char* title, const char* msg, uint32_t code, uint32_t addr){
  volatile uint16_t* vga = (uint16_t*)0xB8000;
  const int W = 80, H = 25; uint8_t color = (4 << 4) | 15; // red bg, white fg
  for(int y=0;y<H;y++) for(int x=0;x<W;x++) vga[y*W+x] = ((uint16_t)color<<8) | ' ';
  const char* p1 = title; int x=2, y=1; while(*p1){ vga[y*W + x++] = ((uint16_t)color<<8) | (uint8_t)*p1++; }
  const char* p2 = msg;   x=2; y=3; while(*p2){ vga[y*W + x++] = ((uint16_t)color<<8) | (uint8_t)*p2++; }
  // leave room for more info lines below
  serial_write("[PANIC] "); serial_write(title); serial_write(" "); serial_write(msg); serial_write("\n");
  serial_write("code="); serial_write_hex(code); serial_write(" addr="); serial_write_hex(addr); serial_write("\n");
}

void panic(const char* message){
  vga_panic_screen("Kernel Panic", message, 0, 0);
  __asm__ __volatile__("cli");
  for(;;){ __asm__ __volatile__("hlt"); }
}

static void timer_tick(void){
  ticks++;
  if ((ticks % 100) == 0){
    serial_write("[tick] "); serial_write_dec(ticks); serial_write("\n");
  }
}

static void pic_unmask_timer_keyboard(void){
  outb(0x21, 0xFC); // enable IRQ0,1
  outb(0xA1, 0xFF);
}

static void dump_registers(const struct isr_context* ctx){
  serial_write("EAX="); serial_write_hex(ctx->eax);
  serial_write(" ECX="); serial_write_hex(ctx->ecx);
  serial_write(" EDX="); serial_write_hex(ctx->edx);
  serial_write(" EBX="); serial_write_hex(ctx->ebx); serial_write("\n");
  serial_write("ESP="); serial_write_hex(ctx->esp);
  serial_write(" EBP="); serial_write_hex(ctx->ebp);
  serial_write(" ESI="); serial_write_hex(ctx->esi);
  serial_write(" EDI="); serial_write_hex(ctx->edi); serial_write("\n");
}

void exception_handler(uint32_t vector, uint32_t error_code, const struct isr_context* ctx){
  uint32_t cr2 = 0;
  if (vector == 14){ __asm__ __volatile__("mov %%cr2, %0" : "=r"(cr2)); }
  // The CPU pushed (in order): error_code (if any), EIP, CS, EFLAGS, [ESP, SS] if privilege change.
  // We cannot reliably read EIP/CS from C without the full stack frame; log what we can.
  serial_write("[EXC] vector="); serial_write_dec(vector);
  serial_write(" err="); serial_write_hex(error_code);
  if (vector == 14){ serial_write(" cr2="); serial_write_hex(cr2); }
  serial_write("\n");
  if (ctx){ dump_registers(ctx); }

  vga_panic_screen("CPU Exception", "System halted.", error_code, cr2);
  __asm__ __volatile__("cli");
  for(;;){ __asm__ __volatile__("hlt"); }
}

// Initialization hook to be called after idt_init()
void irq_init_basic(void){
  irq_install_handler(0, timer_tick);
  irq_install_handler(1, keyboard_irq_handler);
  // PIT 100Hz
  uint32_t freq = 100; uint32_t div = 1193180 / freq;
  outb(0x43, 0x36);
  outb(0x40, div & 0xFF);
  outb(0x40, (div >> 8) & 0xFF);
  pic_unmask_timer_keyboard();
}
