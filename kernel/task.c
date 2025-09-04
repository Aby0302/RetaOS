#include <kernel/task.h>
#include <arch/x86/paging.h>
#include <memory/heap.h>
#include <stdint.h>
#include <stddef.h>

// For inline assembly
#define __ASSEMBLY__

// İlk kullanıcı işlemi için statik bellek
static pcb_t initial_pcb __attribute__((aligned(4096)));

// Kullanıcı moduna geçiş yapmak için assembly yardımcı fonksiyonu
extern void usermode_jump(uint32_t eip, uint32_t esp, uint32_t eflags);

// Kullanıcı moduna geçiş yap (ring3 iret)
void switch_to_usermode(uint32_t entry_point, uint32_t stack_top) {
    __asm__ __volatile__(
        "cli\n\t"
        // Load user data segments (RPL=3)
        "mov $0x2B, %%ax\n\t"
        "mov %%ax, %%ds\n\t"
        "mov %%ax, %%es\n\t"
        "mov %%ax, %%fs\n\t"
        "mov %%ax, %%gs\n\t"
        // Build iret frame: SS, ESP, EFLAGS, CS, EIP
        "pushl $0x2B\n\t"          // SS (user data)
        "pushl %0\n\t"             // ESP
        "pushfl\n\t"
        "popl %%eax\n\t"
        "orl $0x200, %%eax\n\t"     // IF=1
        "pushl %%eax\n\t"
        "pushl $0x23\n\t"          // CS (user code)
        "pushl %1\n\t"             // EIP
        "iret\n\t"
        :
        : "r"(stack_top), "r"(entry_point)
        : "eax", "memory");
}

// İlk kullanıcı işlemini başlat
void init_first_process(void) {
    // Basit implementasyon - gerçek sayfa yönetimi için genişletilmeli
    // TODO: Implement proper page directory creation and mapping
    
    // Kullanıcı bağlamını ayarla (basit)
    initial_pcb.uc.eip = 0x08048000;  // Giriş noktası
    initial_pcb.uc.esp = 0xE0000000;  // Yığın tepe noktası
    initial_pcb.uc.eflags = 0x200;    // IF=1
    initial_pcb.uc.cs = 0x23;         // Kullanıcı kodu segmenti
    initial_pcb.uc.ss = 0x2B;         // Kullanıcı yığın segmenti
    initial_pcb.uc.ds = 0x2B;         // Kullanıcı veri segmentleri
    initial_pcb.uc.es = 0x2B;
    initial_pcb.uc.fs = 0x2B;
    initial_pcb.uc.gs = 0x2B;
    
    // Kullanıcı moduna geçiş yap
    switch_to_usermode(initial_pcb.uc.eip, initial_pcb.uc.esp);
}
