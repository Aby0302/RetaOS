#include <stdint.h>
#include <stddef.h>

struct __attribute__((packed)) gdt_entry {
  uint16_t limit_low; uint16_t base_low; uint8_t base_mid;
  uint8_t access; uint8_t gran; uint8_t base_hi;
};
struct __attribute__((packed)) gdt_ptr { uint16_t limit; uint32_t base; };

// Minimal 32-bit TSS structure
struct __attribute__((packed)) tss_entry {
  uint32_t prev_tss;   uint32_t esp0; uint32_t ss0;
  uint32_t esp1;       uint32_t ss1;  uint32_t esp2; uint32_t ss2;
  uint32_t cr3;        uint32_t eip;  uint32_t eflags;
  uint32_t eax, ecx, edx, ebx;
  uint32_t esp, ebp, esi, edi;
  uint32_t es; uint32_t cs; uint32_t ss; uint32_t ds; uint32_t fs; uint32_t gs;
  uint32_t ldt;        uint16_t trap; uint16_t iomap_base;
};

static struct gdt_entry gdt[4];
static struct gdt_ptr gp;
static struct tss_entry tss;

extern void gdt_flush(uint32_t gp_addr);

static void gdt_set(int i, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran){
  gdt[i].limit_low = (limit & 0xFFFF);
  gdt[i].base_low  = (base & 0xFFFF);
  gdt[i].base_mid  = (base >> 16) & 0xFF;
  gdt[i].access    = access;
  gdt[i].gran      = ((limit >> 16) & 0x0F) | (gran & 0xF0);
  gdt[i].base_hi   = (base >> 24) & 0xFF;
}

static void write_tss(int num, uint16_t ss0, uint32_t esp0){
  // Zero the TSS (byte-wise to avoid packed-struct alignment warnings)
  uint8_t* t = (uint8_t*)&tss;
  for (size_t i = 0; i < (size_t)sizeof(tss); ++i) t[i] = 0;

  uint32_t base = (uint32_t)&tss;
  uint32_t limit = sizeof(tss) - 1;
  // 0x89 = present | DPL=0 | type=Available 32-bit TSS
  gdt_set(num, base, limit, 0x89, 0x00);

  tss.ss0 = ss0; tss.esp0 = esp0;
  // Set segment selectors (ring0 code/data)
  tss.cs = 0x08; // GDT entry 1, RPL 0
  tss.ds = tss.es = tss.fs = tss.gs = tss.ss = 0x10; // GDT entry 2, RPL 0
  tss.iomap_base = sizeof(tss);
}

// Expose a small API to update the kernel stack top used on privilege transitions
void tss_set_kernel_stack(uint32_t esp0){ tss.esp0 = esp0; }

void gdt_init(void){
  gp.limit = sizeof(gdt) - 1; gp.base  = (uint32_t)&gdt;
  gdt_set(0,0,0,0,0);
  gdt_set(1,0,0xFFFFF,0x9A,0xCF); /* kernel code */
  gdt_set(2,0,0xFFFFF,0x92,0xCF); /* kernel data */
  // Setup TSS at selector 0x18 (index 3)
  write_tss(3, 0x10, 0x0);

  gdt_flush((uint32_t)&gp);

  // Load TR with our TSS selector (0x18)
  uint16_t tss_sel = 0x18;
  __asm__ __volatile__("ltr %0" : : "r"(tss_sel));
}
