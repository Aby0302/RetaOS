#include <stdint.h>
struct __attribute__((packed)) idt_entry{
  uint16_t base_lo; uint16_t sel; uint8_t always0; uint8_t flags; uint16_t base_hi;
};
struct __attribute__((packed)) idt_ptr{ uint16_t limit; uint32_t base; };
extern void isr14_stub(void);
static struct idt_entry idt[256]; static struct idt_ptr ip;
static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags){
  idt[num].base_lo = base & 0xFFFF; idt[num].sel = sel; idt[num].always0 = 0;
  idt[num].flags = flags; idt[num].base_hi = (base >> 16) & 0xFFFF;
}
void idt_load(uint32_t ip_addr);
void idt_init(void){
  ip.limit = sizeof(idt) - 1; ip.base = (uint32_t)&idt;
  for(int i=0;i<256;i++) idt_set_gate(i,0,0x08,0x8E);
  idt_set_gate(14,(uint32_t)isr14_stub,0x08,0x8E);
  idt_load((uint32_t)&ip);
}
