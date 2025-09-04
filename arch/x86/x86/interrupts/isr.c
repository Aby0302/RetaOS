#include "../../../../include/drivers/serial.h"
#include "../../../../include/arch/x86/io.h"
#include "../../../../include/arch/x86/isr.h"
#include "../../../../include/kernel/sched.h"
#include "../../../../include/kernel/bsod.h"
#include "../../../../include/kernel/irq.h"
#include <stdint.h>

// forward decls from drivers
void keyboard_irq_handler(void);

static volatile unsigned int ticks = 0;

void panic(const char* message){
  kernel_bsod("%s", message);
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

  kernel_bsod_exception(vector, error_code, ctx, cr2);
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

// IRQ0 handler used by irq0_stub for preemption.
void irq0_handler(uint32_t isr_esp) {
  timer_tick();
  // scheduler_on_timer_isr(isr_esp); // Temporarily disabled to prevent triple fault
}

// timer_get_ticks is now defined in kernel/timer.c
