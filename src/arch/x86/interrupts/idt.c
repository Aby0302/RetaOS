#include <stdint.h>
#include "include/arch/x86/io.h"
#include "include/arch/x86/isr.h"

struct __attribute__((packed)) idt_entry{
  uint16_t base_lo; uint16_t sel; uint8_t always0; uint8_t flags; uint16_t base_hi;
};
struct __attribute__((packed)) idt_ptr{ uint16_t limit; uint32_t base; };

extern void ex0_stub(void);  extern void ex1_stub(void);  extern void ex2_stub(void);  extern void ex3_stub(void);
extern void ex4_stub(void);  extern void ex5_stub(void);  extern void ex6_stub(void);  extern void ex7_stub(void);
extern void ex8_stub(void);  extern void ex9_stub(void);  extern void ex10_stub(void); extern void ex11_stub(void);
extern void ex12_stub(void); extern void ex13_stub(void); extern void ex14_stub_pf(void); extern void ex15_stub(void);
extern void ex16_stub(void); extern void ex17_stub(void); extern void ex18_stub(void); extern void ex19_stub(void);

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

  // CPU exceptions 0..19
  idt_set_gate(0,(uint32_t)ex0_stub,0x08,0x8E);
  idt_set_gate(1,(uint32_t)ex1_stub,0x08,0x8E);
  idt_set_gate(2,(uint32_t)ex2_stub,0x08,0x8E);
  idt_set_gate(3,(uint32_t)ex3_stub,0x08,0x8E);
  idt_set_gate(4,(uint32_t)ex4_stub,0x08,0x8E);
  idt_set_gate(5,(uint32_t)ex5_stub,0x08,0x8E);
  idt_set_gate(6,(uint32_t)ex6_stub,0x08,0x8E);
  idt_set_gate(7,(uint32_t)ex7_stub,0x08,0x8E);
  idt_set_gate(8,(uint32_t)ex8_stub,0x08,0x8E);
  idt_set_gate(9,(uint32_t)ex9_stub,0x08,0x8E);
  idt_set_gate(10,(uint32_t)ex10_stub,0x08,0x8E);
  idt_set_gate(11,(uint32_t)ex11_stub,0x08,0x8E);
  idt_set_gate(12,(uint32_t)ex12_stub,0x08,0x8E);
  idt_set_gate(13,(uint32_t)ex13_stub,0x08,0x8E);
  idt_set_gate(14,(uint32_t)ex14_stub_pf,0x08,0x8E);
  idt_set_gate(15,(uint32_t)ex15_stub,0x08,0x8E);
  idt_set_gate(16,(uint32_t)ex16_stub,0x08,0x8E);
  idt_set_gate(17,(uint32_t)ex17_stub,0x08,0x8E);
  idt_set_gate(18,(uint32_t)ex18_stub,0x08,0x8E);
  idt_set_gate(19,(uint32_t)ex19_stub,0x08,0x8E);

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
