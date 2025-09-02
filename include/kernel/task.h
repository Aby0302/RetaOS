#ifndef _KERNEL_TASK_H
#define _KERNEL_TASK_H

#include <stdint.h>

// Kullanıcı modu bağlam yapısı
struct user_context {
    uint32_t eip;
    uint32_t esp;
    uint32_t eflags;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t es;
    uint32_t fs;
    uint32_t gs;
} __attribute__((packed));

// İşlem kontrol bloğu (PCB) yapısı
typedef struct {
    uint32_t pid;               // İşlem kimliği
    uint32_t esp;               // Çekirdek yığın göstericisi
    uint32_t page_directory;    // Sayfa dizini fiziksel adresi
    struct user_context uc;      // Kullanıcı modu bağlamı
    // Diğer işlem bilgileri...
} pcb_t;

// İlk kullanıcı işlemini başlat
void init_first_process(void);

// Kullanıcı moduna geçiş yap
void switch_to_usermode(uint32_t entry_point, uint32_t stack_top);

#endif // _KERNEL_TASK_H
