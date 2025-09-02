#include "include/arch/x86/acpi.h"
#include "include/drivers/serial.h"
#include <stdint.h>

static const acpi_rsdp_t* g_rsdp = 0;
static const acpi_sdt_header_t* g_rsdt = 0;
static const acpi_sdt_header_t* g_xsdt = 0;
static const acpi_madt_t* g_madt = 0;
static const acpi_hpet_t* g_hpet = 0;

static int memcmp_(const void* a, const void* b, uint32_t n){
    const unsigned char* p=a; const unsigned char* q=b;
    for (uint32_t i=0;i<n;i++){ if (p[i]!=q[i]) return (int)p[i] - (int)q[i]; }
    return 0;
}

static uint8_t checksum8(const uint8_t* p, uint32_t len){
    uint32_t s=0; for (uint32_t i=0;i<len;i++) s += p[i]; return (uint8_t)(s & 0xFF);
}

int acpi_table_checksum_ok(const void* table, uint32_t length){
    return checksum8((const uint8_t*)table, length) == 0;
}

static const acpi_rsdp_t* scan_rsdp_area(uint32_t start, uint32_t end){
    // scan 16-byte aligned
    for (uint32_t p=start; p<end; p+=16){
        const acpi_rsdp_t* rsdp = (const acpi_rsdp_t*)(uintptr_t)p;
        if (memcmp_((const void*)rsdp->signature, "RSD PTR ", 8) == 0){
            // verify checksum(s)
            if (rsdp->revision == 0){
                if (checksum8((const uint8_t*)rsdp, 20) == 0) return rsdp;
            } else {
                if (checksum8((const uint8_t*)rsdp, 20) == 0 && checksum8((const uint8_t*)rsdp, rsdp->length) == 0) return rsdp;
            }
        }
    }
    return 0;
}

static const acpi_rsdp_t* find_rsdp(void){
    // Option 1: EBDA segment at 0x40E (BDA)
    uint16_t* ebda_seg_ptr = (uint16_t*)(uintptr_t)0x040E;
    uint32_t ebda_addr = ((uint32_t)(*ebda_seg_ptr)) << 4;
    if (ebda_addr){
        const acpi_rsdp_t* r = scan_rsdp_area(ebda_addr, ebda_addr + 0x4000);
        if (r) return r;
    }
    // Option 2: BIOS area 0xE0000 - 0xFFFFF
    return scan_rsdp_area(0xE0000, 0x100000);
}

static void discover_sdt_pointers(void){
    if (!g_rsdp) return;
    if (g_rsdp->revision >= 2 && g_rsdp->xsdt_address){
        g_xsdt = (const acpi_sdt_header_t*)(uintptr_t)g_rsdp->xsdt_address;
        if (g_xsdt && !acpi_table_checksum_ok(g_xsdt, g_xsdt->length)){
            serial_write("[ACPI] XSDT checksum bad, ignoring.\n");
            g_xsdt = 0;
        }
    }
    if (!g_xsdt){
        g_rsdt = (const acpi_sdt_header_t*)(uintptr_t)g_rsdp->rsdt_address;
        if (g_rsdt && !acpi_table_checksum_ok(g_rsdt, g_rsdt->length)){
            serial_write("[ACPI] RSDT checksum bad, ignoring.\n");
            g_rsdt = 0;
        }
    }
}

static const acpi_sdt_header_t* find_table_in_rsdt(const char sig[4]){
    if (!g_rsdt) return 0;
    uint32_t count = (g_rsdt->length - sizeof(acpi_sdt_header_t)) / 4u;
    const uint32_t* ents = (const uint32_t*)((const uint8_t*)g_rsdt + sizeof(acpi_sdt_header_t));
    for (uint32_t i=0;i<count;i++){
        const acpi_sdt_header_t* h = (const acpi_sdt_header_t*)(uintptr_t)ents[i];
        if (h && memcmp_(h->signature, sig, 4) == 0 && acpi_table_checksum_ok(h, h->length)) return h;
    }
    return 0;
}

static const acpi_sdt_header_t* find_table_in_xsdt(const char sig[4]){
    if (!g_xsdt) return 0;
    uint32_t count = (g_xsdt->length - sizeof(acpi_sdt_header_t)) / 8u;
    const uint64_t* ents = (const uint64_t*)((const uint8_t*)g_xsdt + sizeof(acpi_sdt_header_t));
    for (uint32_t i=0;i<count;i++){
        const acpi_sdt_header_t* h = (const acpi_sdt_header_t*)(uintptr_t)ents[i];
        if (h && memcmp_(h->signature, sig, 4) == 0 && acpi_table_checksum_ok(h, h->length)) return h;
    }
    return 0;
}

static void locate_madt_hpet(void){
    g_madt = 0; g_hpet = 0;
    if (g_xsdt){
        g_madt = (const acpi_madt_t*)find_table_in_xsdt("APIC");
        g_hpet = (const acpi_hpet_t*)find_table_in_xsdt("HPET");
    }
    if ((!g_madt || !g_hpet) && g_rsdt){
        if (!g_madt) g_madt = (const acpi_madt_t*)find_table_in_rsdt("APIC");
        if (!g_hpet) g_hpet = (const acpi_hpet_t*)find_table_in_rsdt("HPET");
    }
}

void acpi_init(void){
    g_rsdp = find_rsdp();
    if (!g_rsdp){ serial_write("[ACPI] RSDP not found.\n"); return; }
    serial_write("[ACPI] RSDP found. Rev=");
    serial_write_dec((unsigned)g_rsdp->revision);
    serial_write(" OEM=");
    {
        char oem[7];
        for (int i=0;i<6;i++){ char ch=g_rsdp->oem_id[i]; if (!ch) ch=' '; oem[i]=ch; }
        oem[6]='\0';
        serial_write(oem);
    }
    serial_write("\n");

    discover_sdt_pointers();
    if (g_xsdt){ serial_write("[ACPI] XSDT at "); serial_write_hex((unsigned)(uintptr_t)g_xsdt); serial_write("\n"); }
    if (g_rsdt){ serial_write("[ACPI] RSDT at "); serial_write_hex((unsigned)(uintptr_t)g_rsdt); serial_write("\n"); }

    locate_madt_hpet();
    if (g_madt){ serial_write("[ACPI] MADT at "); serial_write_hex((unsigned)(uintptr_t)g_madt); serial_write(" LAPIC@ "); serial_write_hex(g_madt->local_apic_addr); serial_write("\n"); }
    else { serial_write("[ACPI] MADT not found.\n"); }
    if (g_hpet){ serial_write("[ACPI] HPET at "); serial_write_hex((unsigned)(uintptr_t)g_hpet); serial_write(" MMIO@ "); serial_write_hex((unsigned)(uintptr_t)g_hpet->address.address); serial_write("\n"); }
    else { serial_write("[ACPI] HPET not found.\n"); }
}

const acpi_rsdp_t* acpi_get_rsdp(void){ return g_rsdp; }
const acpi_sdt_header_t* acpi_get_rsdt(void){ return g_rsdt; }
const acpi_sdt_header_t* acpi_get_xsdt(void){ return g_xsdt; }
const acpi_madt_t* acpi_find_madt(void){ return g_madt; }
const acpi_hpet_t* acpi_find_hpet(void){ return g_hpet; }
