#include <kernel/elf.h>
#include <arch/x86/paging.h>
#include <memory/heap.h>
#include <kernel/task.h>
#include <kernel/vfs.h>
#include <kernel/process.h>
#include <kernel/types.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "drivers/serial.h" // Ensure serial_write functions are declared

// Forward declarations for missing functions
extern void* kmalloc_a(unsigned long size);
extern void map_page(page_directory_t* dir, void* virt, void* phys, int flags);

// ELF dosyasının geçerli olup olmadığını kontrol et
static int elf_validate(const Elf32_Ehdr* hdr) {
    // Sihirli sayıyı kontrol et (0x7F + "ELF")
    if (hdr->e_ident[0] != 0x7F || 
        hdr->e_ident[1] != 'E' || 
        hdr->e_ident[2] != 'L' || 
        hdr->e_ident[3] != 'F') {
        return 0; // Geçersiz ELF dosyası
    }
    
    // 32-bit ELF dosyası mı?
    if (hdr->e_ident[4] != 1) { // ELFCLASS32
        return 0;
    }
    
    // Little-endian mı?
    if (hdr->e_ident[5] != 1) { // ELFDATA2LSB
        return 0;
    }
    
    // Geçerli versiyon mu?
    if (hdr->e_ident[6] != 1) { // EV_CURRENT
        return 0;
    }
    
    // x86 mimarisi mi?
    if (hdr->e_machine != 3) { // EM_386
        return 0;
    }
    
    return 1; // Geçerli ELF dosyası
}

// Bellek sayfalarını eşle
static void map_pages(page_directory_t* page_dir, void* virt_addr, size_t size, int user) {
    uint32_t virt = (uint32_t)virt_addr;
    uint32_t end = virt + size;
    
    // Sayfa hizalamasını yap
    virt = virt & ~0xFFF;
    
    // Her sayfayı eşle
    for (; virt < end; virt += 0x1000) {
        void* phys = kmalloc_a(0x1000);
        if (phys) {
            map_page(page_dir, (void*)virt, phys, user ? 1 : 0);
        }
    }
}

// ELF dosyasını yükle
void* elf_load(const void* data, size_t size) {
    (void)data; // Suppress unused parameter warning
    (void)size; // Suppress unused parameter warning

    struct stat st; // Use correct structure for vfs_stat
    if (vfs_stat("/bin/init.elf", &st) < 0) {
        serial_write("[elf_exec] Failed to stat /bin/init.elf\n");
        return NULL;
    }
    serial_write("[elf_exec] File size: ");
    
    char size_str[20]; // Buffer to hold the string representation of the size
    snprintf(size_str, sizeof(size_str), "%d", (int)st.st_size); // Convert size to string
    serial_write(size_str); // Pass the string to serial_write
    
    serial_write(" bytes\n");

    if ((size_t)st.st_size < sizeof(Elf32_Ehdr)) { // Cast to size_t to fix signed/unsigned comparison
        serial_write("[elf_exec] File too small to be a valid ELF\n");
        return NULL;
    }

    Elf32_Ehdr eh;
    int fd = vfs_open("/bin/init.elf", 0); // Open the ELF file
    if (fd < 0) {
        serial_write("[elf_exec] Failed to open ELF file\n");
        return NULL;
    }

    ssize_t rd = vfs_read(fd, &eh, sizeof(eh));
    if (rd < (ssize_t)sizeof(eh)) {
        serial_write("[elf_exec] Failed to read ELF header\n");
        vfs_close(fd);
        return NULL;
    }

    serial_write("[elf_exec] ELF magic: ");
    serial_write_hex(eh.e_ident[0]);
    serial_write_hex(eh.e_ident[1]);
    serial_write_hex(eh.e_ident[2]);
    serial_write_hex(eh.e_ident[3]);
    serial_write("\n");

    if (eh.e_ident[0] != 0x7F || eh.e_ident[1] != 'E' || eh.e_ident[2] != 'L' || eh.e_ident[3] != 'F') {
        serial_write("[elf_exec] Invalid ELF magic\n");
        vfs_close(fd);
        return NULL;
    }

    size_t elf_size = (size_t)st.st_size;
    void* elf_data = kmalloc(elf_size);
    if (!elf_data) {
        serial_write("[elf_exec] Memory allocation failed\n");
        vfs_close(fd);
        return NULL;
    }

    rd = vfs_read(fd, elf_data, elf_size);
    if (rd < (ssize_t)elf_size) {
        serial_write("[elf_exec] Failed to read ELF file\n");
        kfree(elf_data);
        vfs_close(fd);
        return NULL;
    }

    vfs_close(fd);

    // Validate ELF file (additional checks can be added here)
    if (eh.e_ident[0] != 0x7F || eh.e_ident[1] != 'E' || eh.e_ident[2] != 'L' || eh.e_ident[3] != 'F') {
        serial_write("[elf_exec] Invalid ELF file\n");
        kfree(elf_data);
        return NULL;
    }

    return elf_data; // Return the loaded ELF data
}

int elf_exec(const char* filename) {
    // Load the ELF file into memory
    struct stat st;
    if (vfs_stat(filename, &st) < 0) {
        serial_write("[elf_exec] Failed to stat file\n");
        return -1;
    }

    if ((size_t)st.st_size < sizeof(Elf32_Ehdr)) {
        serial_write("[elf_exec] File too small to be a valid ELF\n");
        return -1;
    }

    int fd = vfs_open(filename, 0);
    if (fd < 0) {
        serial_write("[elf_exec] Failed to open file\n");
        return -1;
    }

    Elf32_Ehdr eh;
    if (vfs_read(fd, &eh, sizeof(eh)) < (ssize_t)sizeof(eh)) {
        serial_write("[elf_exec] Failed to read ELF header\n");
        vfs_close(fd);
        return -1;
    }

    if (!elf_validate(&eh)) {
        serial_write("[elf_exec] Invalid ELF file\n");
        vfs_close(fd);
        return -1;
    }

    // Allocate memory for the ELF file
    void* elf_data = kmalloc(st.st_size);
    if (!elf_data) {
        serial_write("[elf_exec] Memory allocation failed\n");
        vfs_close(fd);
        return -1;
    }

    if (vfs_read(fd, elf_data, st.st_size) < (ssize_t)st.st_size) {
        serial_write("[elf_exec] Failed to read ELF file\n");
        kfree(elf_data);
        vfs_close(fd);
        return -1;
    }

    vfs_close(fd);

    // Parse program headers and map segments
    Elf32_Phdr* ph = (Elf32_Phdr*)((uint8_t*)elf_data + eh.e_phoff);
    for (int i = 0; i < eh.e_phnum; i++) {
        if (ph[i].p_type != PT_LOAD) {
            continue;
        }

        void* virt_addr = (void*)ph[i].p_vaddr;
        size_t mem_size = ph[i].p_memsz;
        process_t* cur = process_current();
        page_directory_t* dir = cur ? cur->page_dir : NULL;
        map_pages(dir, virt_addr, mem_size, 1);

        memcpy(virt_addr, (uint8_t*)elf_data + ph[i].p_offset, ph[i].p_filesz);
    }

    kfree(elf_data);

    // Set up entry point and switch to user mode
    uint32_t entry = (uint32_t)eh.e_entry;
    // Use a stack within the identity-mapped 0..4MB region
    uint32_t user_stack_top = 0x003FF000; // just below 4MB, page-aligned
    switch_to_usermode(entry, user_stack_top);

    return 0;
}
