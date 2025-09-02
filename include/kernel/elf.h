#ifndef _KERNEL_ELF_H
#define _KERNEL_ELF_H

#include <stdint.h>
#include <stddef.h>  // size_t için

// ELF tanımlamaları
typedef uint32_t Elf32_Addr;
typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Off;
typedef int32_t  Elf32_Sword;
typedef uint32_t Elf32_Word;

// ELF başlık yapısı
#define EI_NIDENT 16
typedef struct {
    unsigned char e_ident[EI_NIDENT];
    Elf32_Half    e_type;
    Elf32_Half    e_machine;
    Elf32_Word    e_version;
    Elf32_Addr    e_entry;
    Elf32_Off     e_phoff;
    Elf32_Off     e_shoff;
    Elf32_Word    e_flags;
    Elf32_Half    e_ehsize;
    Elf32_Half    e_phentsize;
    Elf32_Half    e_phnum;
    Elf32_Half    e_shentsize;
    Elf32_Half    e_shnum;
    Elf32_Half    e_shstrndx;
} __attribute__((packed)) Elf32_Ehdr;

// Program başlık girişi yapısı
typedef struct {
    Elf32_Word p_type;
    Elf32_Off  p_offset;
    Elf32_Addr p_vaddr;
    Elf32_Addr p_paddr;
    Elf32_Word p_filesz;
    Elf32_Word p_memsz;
    Elf32_Word p_flags;
    Elf32_Word p_align;
} __attribute__((packed)) Elf32_Phdr;

// Segment tipleri
#define PT_NULL    0
#define PT_LOAD    1
#define PT_DYNAMIC 2
#define PT_INTERP  3
#define PT_NOTE    4

// Segment izin bayrakları
#define PF_X 0x1
#define PF_W 0x2
#define PF_R 0x4

// ELF dosyasını yükle ve çalıştır
// Parametreler:
//   - data: ELF dosyasının bellek adresi
//   - size: Dosya boyutu
// Dönüş değeri: Giriş noktası adresi veya NULL (hata durumunda)
void* elf_load(const void* data, size_t size);

// ELF dosyasını yükle ve yeni bir işlem olarak başlat
// Parametreler:
//   - filename: Yüklenecek ELF dosyasının adı
// Dönüş değeri: İşlem kimliği veya -1 (hata durumunda)
int elf_exec(const char* filename);

#endif // _KERNEL_ELF_H
