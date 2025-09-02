#include <stdint.h>
#include <stddef.h>

// Global variables to store Multiboot2 information
static uint64_t fb_addr = 0;
static uint32_t fb_width = 0;
static uint32_t fb_height = 0;
static uint32_t fb_pitch = 0;
static uint8_t fb_bpp = 0;

static void* elf_symtab = NULL;
static uint32_t elf_num = 0;
static uint32_t elf_size = 0;
static uint32_t elf_shndx = 0;

static uint16_t apm_version = 0;
static uint16_t apm_cseg = 0;
static uint32_t apm_offset = 0;
static uint16_t apm_cseg_16 = 0;
static uint16_t apm_dseg = 0;
static uint16_t apm_flags = 0;
static uint16_t apm_cseg_len = 0;
static uint16_t apm_cseg_16_len = 0;
static uint16_t apm_dseg_len = 0;

static uint32_t efi_system_table_32_addr = 0;
static uint64_t efi_system_table_64_addr = 0;

static uint8_t smbios_major = 0;
static uint8_t smbios_minor = 0;
static void* smbios_entry_point = NULL;

// Set framebuffer information
void set_framebuffer_info(uint64_t addr, uint32_t width, uint32_t height, uint32_t pitch, uint8_t bpp) {
    fb_addr = addr;
    fb_width = width;
    fb_height = height;
    fb_pitch = pitch;
    fb_bpp = bpp;
}

// Set ELF symbol information
void set_elf_symbols(void* symtab, uint32_t num, uint32_t size, uint32_t shndx) {
    elf_symtab = symtab;
    elf_num = num;
    elf_size = size;
    elf_shndx = shndx;
}

// Set APM information
void set_apm_info(uint16_t version, uint16_t cseg, uint32_t offset, 
                 uint16_t cseg_16, uint16_t dseg, uint16_t flags,
                 uint16_t cseg_len, uint16_t cseg_16_len, uint16_t dseg_len) {
    apm_version = version;
    apm_cseg = cseg;
    apm_offset = offset;
    apm_cseg_16 = cseg_16;
    apm_dseg = dseg;
    apm_flags = flags;
    apm_cseg_len = cseg_len;
    apm_cseg_16_len = cseg_16_len;
    apm_dseg_len = dseg_len;
}

// Set 32-bit EFI system table address
void set_efi_system_table_32(uint32_t ptr) {
    efi_system_table_32_addr = ptr;
}

// Set 64-bit EFI system table address
void set_efi_system_table_64(uint64_t ptr) {
    efi_system_table_64_addr = ptr;
}

// Set SMBIOS entry point
void set_smbios_entry_point(uint8_t major, uint8_t minor, void* entry_point) {
    smbios_major = major;
    smbios_minor = minor;
    smbios_entry_point = entry_point;
}
