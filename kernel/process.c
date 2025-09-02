#include "include/kernel/process.h"
#include "include/arch/x86/paging.h"
#include "include/memory/heap.h"
#include "include/kernel/elf.h"
#include "include/kernel/sched.h"
#include "include/kernel/task.h"
#include "include/kernel/vfs.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// Forward declare kprintf since it's defined in kernel.c
extern void kprintf(const char* fmt, ...);

// VFS function declarations (temporary until properly defined in vfs.h)
int vfs_open(const char* path, int flags);
int vfs_size(int fd);
ssize_t vfs_read(int fd, void* buf, size_t count);  // Match the declaration in vfs.h
int vfs_close(int fd);

// For debugging
#define UNUSED(x) (void)(x)

// Inline assembly helper functions
static inline void cli(void) { __asm__ volatile ("cli"); }
static inline void sti(void) { __asm__ volatile ("sti"); }
static inline void hlt(void) { __asm__ volatile ("hlt"); }

// External declarations from other modules
extern page_directory_t* kernel_directory;

// Forward declarations
static void switch_page_directory(page_directory_t* dir);
static void clone_page_directory(page_directory_t* dir);
static void* kmalloc_a(size_t size);

// İşlem listesi başı
static process_t* process_list = NULL;
static process_t* current_process = NULL;
static pid_t next_pid = 1;

// Align memory allocation
static void* kmalloc_a(size_t size) {
    // Simple implementation - should be replaced with proper aligned allocation
    return kmalloc(size);
}

// Switch page directory
static void switch_page_directory(page_directory_t* dir) {
    // Implementation should be in paging.c
    (void)dir; // Suppress unused parameter warning
}

// Clone page directory (stub implementation)
static page_directory_t* clone_directory(page_directory_t* src) {
    // For now, just return the source directory
    // A proper implementation would create a new page directory and copy the entries
    UNUSED(src);
    return kernel_directory;
}

// Map a page (stub implementation)
static void map_page(page_directory_t* dir, void* virt, void* phys, int flags) {
    // Implementation should be in paging.c
    UNUSED(dir);
    UNUSED(virt);
    UNUSED(phys);
    UNUSED(flags);
}

// Boş sayfa dizini oluştur
static page_directory_t* create_empty_page_dir(void) {
    // For now, just return a copy of the kernel directory
    // This should be replaced with proper page directory creation
    return kernel_directory;
}

// İşlem yönetimini başlat
void process_init(void) {
    // Kök işlemi oluştur (kernel)
    process_t* init = (process_t*)kmalloc(sizeof(process_t));
    if (!init) return;
    
    memset(init, 0, sizeof(process_t));
    init->pid = next_pid++;
    init->state = PROC_RUNNING;
    init->page_dir = kernel_directory; // Kernel sayfa dizinini kullan
    
    current_process = init;
    process_list = init;
    
    //kprintf("[process] Initialized process manager, init pid=%d\n", init->pid);
}

// Yeni bir işlem oluştur (fork benzeri)
process_t* process_create(void) {
    process_t* parent = current_process;
    process_t* child = (process_t*)kmalloc(sizeof(process_t));
    if (!child) return NULL;
    
    memset(child, 0, sizeof(process_t));
    
    // Temel bilgileri ayarla
    child->pid = next_pid++;
    child->parent_pid = parent ? parent->pid : 0;
    child->state = PROC_NEW;
    
    // Sayfa dizinini klonla
    child->page_dir = clone_directory(parent ? parent->page_dir : kernel_directory);
    if (!child->page_dir) {
        kfree(child);
        return NULL;
    }
    
    // İşlem listesine ekle
    child->next = process_list;
    process_list = child;
    
    //kprintf("[process] Created new process pid=%d\n", child->pid);
    return child;
}

// Bir işlemi çalıştır (exec benzeri)
int process_exec(process_t* proc, const char* path) {
    if (!proc || !path) return -1;
    
    // Open the file
    int fd = vfs_open(path, 0);
    if (fd < 0) {
        //kprintf("[process] Failed to open file: %s\n", path);
        return -1;
    }
    
    // Get file size
    size_t size = vfs_size(fd);
    if (size == 0) {
        vfs_close(fd);
        //kprintf("[process] Empty file: %s\n", path);
        return -1;
    }
    
    // Read file into memory
    void* file_data = kmalloc(size);
    if (!file_data) {
        vfs_close(fd);
        //kprintf("[process] Out of memory for file data\n");
        return -1;
    }
    
    if (vfs_read(fd, file_data, size) != size) {
        kfree(file_data);
        vfs_close(fd);
        //kprintf("[process] Failed to read file: %s\n", path);
        return -1;
    }
    
    vfs_close(fd);
    
    // Load ELF from memory
    void* entry = elf_load(file_data, size);
    kfree(file_data);
    
    if (!entry) {
        //kprintf("[process] Failed to load ELF: %s\n", path);
        return -1;
    }
    
    // Initialize user context
    memset(&proc->uc, 0, sizeof(proc->uc));
    proc->uc.eip = (uint32_t)entry;
    proc->uc.eflags = 0x200; // IF=1
    proc->uc.cs = 0x1B;       // Kullanıcı kodu segmenti (RPL=3)
    proc->uc.ss = 0x23;       // Kullanıcı veri segmenti (RPL=3)
    proc->uc.ds = 0x23;
    proc->uc.es = 0x23;
    proc->uc.fs = 0x23;
    proc->uc.gs = 0x23;
    
    // Set up user stack (1MB area)
    uint32_t user_stack_top = 0xE0000000; // Around 3.5GB
    uint32_t user_stack_size = 0x100000;  // 1MB stack
    
    // Map user stack pages
    for (uint32_t i = 0; i < user_stack_size; i += 0x1000) {
        void* phys = kmalloc_a(0x1000);
        if (!phys) {
            //kprintf("[process] Out of memory for user stack\n");
            return -1;
        }
        map_page(proc->page_dir, (void*)(user_stack_top + i), phys, 1);
    }
    
    proc->uc.esp = user_stack_top + user_stack_size - 16; // Stack top
    proc->state = PROC_READY;
    
    //kprintf("[process] Loaded ELF %s at 0x%x\n", path, entry);
    return 0;
}

// Mevcut işlemi sonlandır
void process_exit(int exit_code) {
    process_t* proc = process_current();
    if (!proc) return;
    
    proc->exit_code = exit_code;
    proc->state = PROC_TERMINATED;
    
    // Eğer çocuk işlemler varsa onları da sonlandır
    process_t* p = process_list;
    while (p) {
        if (p->parent_pid == proc->pid) {
            p->parent_pid = 1; // init sürecine ata
        }
        p = p->next;
    }
    
    // Kaynakları serbest bırak
    // TODO: Sayfa tablolarını, dosya tanıtıcılarını serbest bırak
    
    // Eğer init süreci sonlanıyorsa, sistem durumunu değiştir
    if (proc->pid == 1) {
        //kprintf("[process] init process (pid=1) exited with code %d\n", exit_code);
        // Sonsuz döngüye gir
        cli();
        for(;;) hlt();
    }
    
    // Başka bir işleme geç
    process_schedule();
    
    // Buraya ulaşılmamalı
    for(;;) hlt();
}

// Context switch between processes
__attribute__((noreturn)) void process_switch(process_t* next) {
    if (!next) {
        //kprintf("process_switch: NULL process!\n");
        for(;;) hlt();
    }
    
    // Get current process (if any)
    process_t* current = current_process;
    
    // Save current context if we have one
    if (current) {
        // Save EFLAGS
        uint32_t eflags;
        __asm__ __volatile__ ("pushfl; popl %0" : "=r" (eflags));
        current->uc.eflags = eflags;
        
        // Save stack pointer
        __asm__ __volatile__ ("movl %%esp, %0" : "=r" (current->uc.esp));
        
        // Save segment registers
        uint16_t ds, es, fs, gs;
        __asm__ __volatile__ (
            "mov %%ds, %0\n\t"
            "mov %%es, %1\n\t"
            "mov %%fs, %2\n\t"
            "mov %%gs, %3\n\t"
            : "=r" (ds), "=r" (es), "=r" (fs), "=r" (gs)
        );
        current->uc.ds = ds;
        current->uc.es = es;
        current->uc.fs = fs;
        current->uc.gs = gs;
    }
    
    // Update current process pointer
    current_process = next;
    
    // Load new process context
    uint32_t new_esp = next->uc.esp;
    uint32_t new_eip = next->uc.eip;
    
    // Switch to the new process
    __asm__ __volatile__ (
        // Load new stack pointer
        "movl %0, %%esp\n\t"
        
        // Load segment registers with user data selector (0x23)
        "movw $0x23, %%ax\n\t"
        "movw %%ax, %%ds\n\t"
        "movw %%ax, %%es\n\t"
        "movw %%ax, %%fs\n\t"
        "movw %%ax, %%gs\n\t"
        
        // Push iret frame (ss, esp, eflags, cs, eip)
        "pushl $0x23\n\t"  // ss
        "pushl %0\n\t"     // esp
        "pushfl\n\t"       // eflags
        "pushl $0x1B\n\t"  // cs
        "pushl %1\n\t"     // eip
        
        // Jump to new process
        "iret"
        : 
        : "r" (new_esp), "r" (new_eip)
        : "memory", "eax"
    );
    
    // We should never get here
    for(;;) hlt();
}

// İşlemleri zamanlayıcı
void process_schedule(void) {
    process_t* next = process_list;
    
    // Çalıştırılabilecek bir işlem bul
    while (next) {
        if (next->state == PROC_READY) {
            break;
        }
        next = next->next;
    }
    
    // Hiç hazır işlem yoksa bekle
    if (!next) {
        //kprintf("[process] No runnable processes, halting\n");
        cli();
        for(;;) hlt();
        return; // Never reached
    }
    
    // İşlem değiştir
    process_switch(next);
}

// Mevcut çalışan işlemi al
process_t* process_current(void) {
    return current_process;
}

// İşlem kimliğine göre işlem bul
process_t* process_find(pid_t pid) {
    process_t* p = process_list;
    while (p) {
        if (p->pid == pid) return p;
        p = p->next;
    }
    return NULL;
}

// İşlemi durdur (beklemeye al)
void process_block(process_t* proc) {
    if (!proc) return;
    if (proc->state == PROC_RUNNING || proc->state == PROC_READY) {
        proc->state = PROC_BLOCKED;
    }
}

// İşlemi devam ettir (beklemeden çıkar)
void process_unblock(process_t* proc) {
    if (!proc) return;
    if (proc->state == PROC_BLOCKED) {
        proc->state = PROC_READY;
    }
}

// İşlem listesini yazdır (hata ayıklama için)
void process_dump_list(void) {
    //kprintf("Process list:\n");
    //kprintf("PID\tSTATE\n");
    //kprintf("----------------\n");
    
    process_t* p = process_list;
    while (p) {
        const char* state;
        switch (p->state) {
            case PROC_NEW: state = "NEW"; break;
            case PROC_RUNNING: state = "RUNNING"; break;
            case PROC_READY: state = "READY"; break;
            case PROC_BLOCKED: state = "BLOCKED"; break;
            case PROC_TERMINATED: state = "TERMINATED"; break;
            default: state = "UNKNOWN";
        }
        
        //kprintf("%d\t%s\n", p->pid, state);
        p = p->next;
    }
}
