#include <stdint.h>
struct __attribute__((packed)) gdt_entry {
  uint16_t limit_low; uint16_t base_low; uint8_t base_mid;
  uint8_t access; uint8_t gran; uint8_t base_hi;
};
struct __attribute__((packed)) gdt_ptr { uint16_t limit; uint32_t base; };
static struct gdt_entry gdt[3]; static struct gdt_ptr gp;
extern void gdt_flush(uint32_t gp_addr);
static void gdt_set(int i, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran){
  gdt[i].limit_low = (limit & 0xFFFF);
  gdt[i].base_low  = (base & 0xFFFF);
  gdt[i].base_mid  = (base >> 16) & 0xFF;
  gdt[i].access    = access;
  gdt[i].gran      = ((limit >> 16) & 0x0F) | (gran & 0xF0);
  gdt[i].base_hi   = (base >> 24) & 0xFF;
}
void gdt_init(void){
  gp.limit = sizeof(gdt) - 1; gp.base  = (uint32_t)&gdt;
  gdt_set(0,0,0,0,0);
  gdt_set(1,0,0xFFFFF,0x9A,0xCF); /* code */
  gdt_set(2,0,0xFFFFF,0x92,0xCF); /* data */
  gdt_flush((uint32_t)&gp);
}
