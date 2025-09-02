#pragma once
#include <stdint.h>

// Basic ACPI structures (packed)
#pragma pack(push,1)
typedef struct {
    char     signature[8]; // "RSD PTR "
    uint8_t  checksum;
    char     oem_id[6];
    uint8_t  revision;     // 0=ACPI 1.0, 2=ACPI 2.0+
    uint32_t rsdt_address; // if rev==0
    // ACPI 2.0+
    uint32_t length;       // total size of this struct
    uint64_t xsdt_address; // 64-bit XSDT
    uint8_t  ext_checksum;
    uint8_t  reserved[3];
} acpi_rsdp_t;

typedef struct {
    char     signature[4];
    uint32_t length;      // total length including header
    uint8_t  revision;
    uint8_t  checksum;
    char     oem_id[6];
    char     oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} acpi_sdt_header_t;

// RSDT: header followed by 32-bit table pointers
// XSDT: header followed by 64-bit table pointers

// MADT (APIC)
typedef struct {
    acpi_sdt_header_t h;
    uint32_t local_apic_addr;
    uint32_t flags;
    // followed by variable records
} acpi_madt_t;

// HPET table
typedef struct {
    acpi_sdt_header_t h;
    uint8_t  hardware_rev_id;
    uint8_t  comparator_count:5;
    uint8_t  counter_size:1;
    uint8_t  reserved:1;
    uint8_t  legacy_replacement:1;
    uint16_t pci_vendor_id;
    struct {
        uint8_t address_space_id; // 0=system memory
        uint8_t register_bit_width;
        uint8_t register_bit_offset;
        uint8_t access_size;      // 0=undefined, 1=byte,2=word,3=dword,4=qword
        uint64_t address;
    } __attribute__((packed)) address;
    uint8_t  hpet_number;
    uint16_t minimum_tick;
    uint8_t  page_protection;
} acpi_hpet_t;
#pragma pack(pop)

// Public API
void acpi_init(void);
const acpi_rsdp_t* acpi_get_rsdp(void);
const acpi_sdt_header_t* acpi_get_rsdt(void);
const acpi_sdt_header_t* acpi_get_xsdt(void);
const acpi_madt_t* acpi_find_madt(void);
const acpi_hpet_t* acpi_find_hpet(void);

// Utility
int acpi_table_checksum_ok(const void* table, uint32_t length);
