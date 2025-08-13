#include <stdint.h>
#include "include/io.h"
#include "include/isr.h"

struct __attribute__((packed)) idt_entry{
  uint16_t base_lo; uint16_t sel; uint8_t always0; uint8_t flags; uint16_t base_hi;
};
struct __attribute__((packed)) idt_ptr{ uint16_t limit; uint32_t base; };

extern void isr14_stub(void);
extern void irq0_stub(void);  extern void irq1_stub(void);
extern void irq2_stub(void);  extern void irq3_stub(void);
extern void irq4_stub(void);  extern void irq5_stub(void);
extern void irq6_stub(void);  extern void irq7_stub(void);
extern void irq8_stub(void);  extern void irq9_stub(void);
extern void irq10_stub(void); extern void irq11_stub(void);
extern void irq12_stub(void); extern void irq13_stub(void);
extern void irq14_stub(void); extern void irq15_stub(void);

static struct idt_entry idt[256]; static struct idt_ptr ip;
static void (*irq_handlers[16])(void);

static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags){
  idt[num].base_lo = base & 0xFFFF; idt[num].sel = sel; idt[num].always0 = 0;
  idt[num].flags = flags; idt[num].base_hi = (base >> 16) & 0xFFFF;
}

void idt_load(uint32_t ip_addr);

static void pic_remap(void){
  // ICW1
  outb(0x20, 0x11); outb(0xA0, 0x11);
  // ICW2: vector offsets
  outb(0x21, 0x20); // master -> 0x20
  outb(0xA1, 0x28); // slave  -> 0x28
  // ICW3: wiring
  outb(0x21, 0x04); // slave at IRQ2
  outb(0xA1, 0x02);
  // ICW4
  outb(0x21, 0x01); outb(0xA1, 0x01);
  // Mask all initially
  outb(0x21, 0xFF); outb(0xA1, 0xFF);
}

void irq_install_handler(uint8_t irq, void (*handler)(void)){
  if (irq < 16) irq_handlers[irq] = handler;
}

void interrupts_enable(void){ __asm__ __volatile__("sti"); }
void interrupts_disable(void){ __asm__ __volatile__("cli"); }

void idt_init(void){
  ip.limit = sizeof(idt) - 1; ip.base = (uint32_t)&idt;
  for(int i=0;i<256;i++) idt_set_gate(i,0,0x08,0x8E);

  // CPU exceptions (example: page fault)
  idt_set_gate(14,(uint32_t)isr14_stub,0x08,0x8E);

  // PIC remap and IRQ gates
  pic_remap();
  idt_set_gate(32,(uint32_t)irq0_stub,0x08,0x8E);
  idt_set_gate(33,(uint32_t)irq1_stub,0x08,0x8E);
  idt_set_gate(34,(uint32_t)irq2_stub,0x08,0x8E);
  idt_set_gate(35,(uint32_t)irq3_stub,0x08,0x8E);
  idt_set_gate(36,(uint32_t)irq4_stub,0x08,0x8E);
  idt_set_gate(37,(uint32_t)irq5_stub,0x08,0x8E);
  idt_set_gate(38,(uint32_t)irq6_stub,0x08,0x8E);
  idt_set_gate(39,(uint32_t)irq7_stub,0x08,0x8E);
  idt_set_gate(40,(uint32_t)irq8_stub,0x08,0x8E);
  idt_set_gate(41,(uint32_t)irq9_stub,0x08,0x8E);
  idt_set_gate(42,(uint32_t)irq10_stub,0x08,0x8E);
  idt_set_gate(43,(uint32_t)irq11_stub,0x08,0x8E);
  idt_set_gate(44,(uint32_t)irq12_stub,0x08,0x8E);
  idt_set_gate(45,(uint32_t)irq13_stub,0x08,0x8E);
  idt_set_gate(46,(uint32_t)irq14_stub,0x08,0x8E);
  idt_set_gate(47,(uint32_t)irq15_stub,0x08,0x8E);

  idt_load((uint32_t)&ip);
}

void irq_handler(uint8_t irq){
  // Send EOI
  if (irq >= 8) outb(0xA0, 0x20);
  outb(0x20, 0x20);
  // Dispatch
  if (irq < 16 && irq_handlers[irq]) irq_handlers[irq]();
}
