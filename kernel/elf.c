#include <kernel/elf.h>
#include <arch/x86/paging.h>
#include <memory/heap.h>
#include <kernel/task.h>
#include <kernel/vfs.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

// Forward declarations for missing functions
extern void* kmalloc_a(unsigned long size);
extern void map_page(uint32_t page_dir, void* virt, void* phys, int user);

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
static void map_pages(uint32_t page_dir, void* virt_addr, size_t size, int user) {
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
    const Elf32_Ehdr* ehdr = (const Elf32_Ehdr*)data;
    
    // ELF başlığını doğrula
    if (!elf_validate(ehdr)) {
        return NULL;
    }
    
    // Program başlıklarını al
    const Elf32_Phdr* phdr = (const Elf32_Phdr*)((uint32_t)data + ehdr->e_phoff);
    
    // Her bir segmenti yükle
    for (int i = 0; i < ehdr->e_phnum; i++) {
        const Elf32_Phdr* ph = &phdr[i];
        
        // Sadece yükleme segmentlerini işle
        if (ph->p_type == PT_LOAD) {
            // Bellek alanını ayır ve eşle
            // TODO: Implement proper page mapping
            // map_pages(current_directory, (void*)ph->p_vaddr, ph->p_memsz, 1);
            
            // Segment verilerini kopyala
            memcpy((void*)ph->p_vaddr, (const void*)((uint32_t)data + ph->p_offset), ph->p_filesz);
            
            // BSS bölümünü sıfırla (eğer varsa)
            if (ph->p_memsz > ph->p_filesz) {
                memset((void*)(ph->p_vaddr + ph->p_filesz), 0, ph->p_memsz - ph->p_filesz);
            }
        }
    }
    
    // Giriş noktasını döndür
    return (void*)ehdr->e_entry;
}

// ELF dosyasını yükle ve yeni bir işlem olarak başlat
int elf_exec(const char* filename) {
    // Dosyayı aç
    int fd = vfs_open(filename, 0);
    if (fd < 0) {
        return -1; // Dosya açılamadı
    }
    
    // Dosya boyutunu al (basit implementasyon)
    // TODO: Implement proper vfs_size function
    size_t size = 0x10000; // Geçici olarak sabit boyut
    if (size < sizeof(Elf32_Ehdr)) {
        vfs_close(fd);
        return -1; // Çok küçük dosya
    }
    
    // Dosyayı belleğe yükle
    void* data = kmalloc(size);
    if (!data) {
        vfs_close(fd);
        return -1; // Bellek yetersiz
    }
    
    // Dosya içeriğini oku
    if (vfs_read(fd, data, size) != size) {
        kfree(data);
        vfs_close(fd);
        return -1; // Okuma hatası
    }
    
    vfs_close(fd);
    
    // ELF dosyasını yükle
    void* entry = elf_load(data, size);
    if (!entry) {
        kfree(data);
        return -1; // Geçersiz ELF dosyası
    }
    
    // Yeni bir işlem başlat (basitçe kullanıcı moduna geçiş yap)
    // NOT: Gerçek bir işlem yönetimi için bu kısım genişletilmeli
    switch_to_usermode((uint32_t)entry, 0xE0000000 + 0x100000 - 16);
    
    kfree(data);
    return 0; // Başarılı
}
