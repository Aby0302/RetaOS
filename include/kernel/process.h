#ifndef _KERNEL_PROCESS_H
#define _KERNEL_PROCESS_H

#include <stdint.h>
#include "task.h"
#include "types.h"  // page_directory_t, pid_t ve diğer özel tipler için

// İşlem durumları
typedef enum {
    PROC_NEW,       // Yeni oluşturuldu
    PROC_RUNNING,   // Çalışıyor
    PROC_READY,     // Çalışmaya hazır
    PROC_BLOCKED,   // Bloke oldu (örneğin I/O bekliyor)
    PROC_TERMINATED // Sonlandı
} process_state_t;

// İşlem kontrol bloğu (PCB)
typedef struct process {
    uint32_t pid;               // İşlem kimliği
    uint32_t parent_pid;        // Ebeveyn işlem kimliği
    process_state_t state;      // İşlem durumu
    uint32_t esp;               // Çekirdek yığın göstericisi
    uint32_t user_esp;          // Kullanıcı yığın göstericisi
    page_directory_t* page_dir; // Sayfa dizini
    struct user_context uc;      // Kullanıcı modu bağlamı
    struct process* next;       // İşlem listesi için sonraki işlem
    int exit_code;              // Çıkış kodu (eğer sonlandıysa)
} process_t;

// İşlem yönetimini başlat
void process_init(void);

// Yeni bir işlem oluştur (fork benzeri)
process_t* process_create(void);

// Bir işlemi çalıştır (exec benzeri)
int process_exec(process_t* proc, const char* path);

// Mevcut işlemi sonlandır
void process_exit(int exit_code);

// İşlemleri zamanlayıcı
void process_schedule(void);

// Mevcut çalışan işlemi al
process_t* process_current(void);

// İşlem kimliğine göre işlem bul
process_t* process_find(pid_t pid);

// İşlemi durdur (beklemeye al)
void process_block(process_t* proc);

// İşlemi devam ettir (beklemeden çıkar)
void process_unblock(process_t* proc);

// İşlem listesini yazdır (hata ayıklama için)
void process_dump_list(void);

#endif // _KERNEL_PROCESS_H
