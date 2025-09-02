#ifndef _ARCH_X86_MULTIBOOT2_H
#define _ARCH_X86_MULTIBOOT2_H

#include <stdint.h>

/* The magic number for the Multiboot2 header. */
#define MULTIBOOT2_HEADER_MAGIC         0xe85250d6

/* The magic number passed by a Multiboot2-compliant boot loader. */
#define MULTIBOOT2_BOOTLOADER_MAGIC 0x2BADB002
#define MULTIBOOT1_BOOTLOADER_MAGIC 0x1BADB002

/* Alignment of multiboot modules. */
#define MULTIBOOT_MOD_ALIGN             0x00001000

/* Alignment of the multiboot info structure. */
#define MULTIBOOT_INFO_ALIGN            0x00000008

/* Types */
typedef unsigned char           multiboot_uint8_t;
typedef unsigned short          multiboot_uint16_t;
typedef unsigned int            multiboot_uint32_t;
typedef unsigned long long      multiboot_uint64_t;

/* The section header table for ELF. */
struct multiboot_elf_section_header_table
{
    multiboot_uint32_t num;
    multiboot_uint32_t size;
    multiboot_uint32_t addr;
    multiboot_uint32_t shndx;
};

/* The Multiboot information. */
struct multiboot_info
{
    /* Multiboot info version number */
    multiboot_uint32_t flags;

    /* Available memory from BIOS */
    multiboot_uint32_t mem_lower;
    multiboot_uint32_t mem_upper;

    /* "root" partition */
    multiboot_uint32_t boot_device;

    /* Kernel command line */
    multiboot_uint32_t cmdline;

    /* Boot-Module list */
    multiboot_uint32_t mods_count;
    multiboot_uint32_t mods_addr;

    /* ELF section header table */
    struct multiboot_elf_section_header_table elf_sec;

    /* Memory Mapping buffer */
    multiboot_uint32_t mmap_length;
    multiboot_uint32_t mmap_addr;

    /* Drive Info buffer */
    multiboot_uint32_t drives_length;
    multiboot_uint32_t drives_addr;

    /* ROM configuration table */
    multiboot_uint32_t config_table;

    /* Boot Loader Name */
    multiboot_uint32_t boot_loader_name;

    /* APM table */
    multiboot_uint32_t apm_table;

    /* Video */
    multiboot_uint32_t vbe_control_info;
    multiboot_uint32_t vbe_mode_info;
    multiboot_uint16_t vbe_mode;
    multiboot_uint16_t vbe_interface_seg;
    multiboot_uint16_t vbe_interface_off;
    multiboot_uint16_t vbe_interface_len;
};

/* The module structure. */
struct multiboot_mod_list
{
    /* the memory used goes from bytes 'mod_start' to 'mod_end-1' inclusive */
    multiboot_uint32_t mod_start;
    multiboot_uint32_t mod_end;

    /* Module command line */
    multiboot_uint32_t cmdline;

    /* padding to take it to 16 bytes (must be zero) */
    multiboot_uint32_t pad;
};

/* Memory map entry structure - using the same format as multiboot2 specification */
struct multiboot_mmap_entry {
    uint64_t addr;    // Starting physical address
    uint64_t len;     // Length of memory region in bytes
    uint32_t type;    // Type of memory region
    uint32_t zero;    // Must be zero
} __attribute__((packed));

typedef struct multiboot_mmap_entry multiboot_memory_map_t;

/* The memory types. */
#define MULTIBOOT_MEMORY_AVAILABLE              1
#define MULTIBOOT_MEMORY_RESERVED               2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE       3
#define MULTIBOOT_MEMORY_NVS                    4
#define MULTIBOOT_MEMORY_BADRAM                 5

/* The following table is the boot information as passed to a multiboot
   operating system. You do not need to do anything with this if you are
   using the multiboot header. */

/* The tag structure */
struct multiboot_tag {
    uint32_t type;
    uint32_t size;
};

/* The list of tags is terminated by a tag of type 0 and size 8. */
#define MULTIBOOT_TAG_TYPE_END          0

/* Indicates a request for information from the boot loader. */
#define MULTIBOOT_TAG_TYPE_CMDLINE      1

/* Indicates a request for the boot loader name. */
#define MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME  2

/* Indicates a request for the module information. */
#define MULTIBOOT_TAG_TYPE_MODULE       3

/* Indicates a request for the basic memory information. */
#define MULTIBOOT_TAG_TYPE_BASIC_MEMINFO  4

/* Indicates a request for the BIOS memory map. */
#define MULTIBOOT_TAG_TYPE_MMAP         6

/* Indicates a request for the VBE information. */
#define MULTIBOOT_TAG_TYPE_VBE          7

/* Indicates a request for the framebuffer information. */
#define MULTIBOOT_TAG_TYPE_FRAMEBUFFER  8

/* Indicates a request for the ELF sections. */
#define MULTIBOOT_TAG_TYPE_ELF_SECTIONS 9

/* Indicates a request for the APM table. */
#define MULTIBOOT_TAG_TYPE_APM          10

/* Indicates a request for the EFI system table. */
#define MULTIBOOT_TAG_TYPE_EFI32        11
#define MULTIBOOT_TAG_TYPE_EFI64        12

/* Indicates a request for the SMBIOS tables. */
#define MULTIBOOT_TAG_TYPE_SMBIOS       13

/* Indicates a request for the ACPI RSDP. */
#define MULTIBOOT_TAG_TYPE_ACPI_OLD     14
#define MULTIBOOT_TAG_TYPE_ACPI_NEW     15

/* Memory map entry is already defined at the top of the file */

/* The tag for the memory map. */
struct multiboot_tag_mmap {
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
    struct multiboot_mmap_entry entries[0];
} __attribute__((packed));

/* The tag for the framebuffer information. */
struct multiboot_tag_framebuffer {
    uint32_t type;
    uint32_t size;
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bpp;
    uint8_t framebuffer_type;
    uint8_t reserved[2];
} __attribute__((packed));

/* The tag for basic memory information. */
struct multiboot_tag_basic_meminfo {
    uint32_t type;
    uint32_t size;
    uint32_t mem_lower;
    uint32_t mem_upper;
};

/* The tag for the command line. */
struct multiboot_tag_string {
    uint32_t type;
    uint32_t size;
    char string[0];
};

// Function declarations for Multiboot2 information storage
void set_framebuffer_info(uint64_t addr, uint32_t width, uint32_t height, uint32_t pitch, uint8_t bpp);
void set_elf_symbols(void* symtab, uint32_t num, uint32_t size, uint32_t shndx);
void set_apm_info(uint16_t version, uint16_t cseg, uint32_t offset, 
                 uint16_t cseg_16, uint16_t dseg, uint16_t flags,
                 uint16_t cseg_len, uint16_t cseg_16_len, uint16_t dseg_len);
void set_efi_system_table_32(uint32_t ptr);
void set_efi_system_table_64(uint64_t ptr);
void set_smbios_entry_point(uint8_t major, uint8_t minor, void* entry_point);

// Function declarations for kernel initialization
void memory_init(uint32_t mb_magic, uint32_t mb_info_addr);
void pic_init(void);
void timer_init(uint32_t frequency);
void fs_init(void);
void syscall_init(void);
void mount_root(void);
void start_init(void);

#endif // _ARCH_X86_MULTIBOOT2_H
