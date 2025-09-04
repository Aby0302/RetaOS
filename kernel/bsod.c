
#include "../include/kernel/console.h"
#include "../include/arch/x86/isr.h"
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>

// Forward decl from console.c
void console_set_cursor(uint32_t x, uint32_t y);

// Exception/interrupt names (0-19)
static const char* s_exc_names[] = {
    "Divide Error",            // 0
    "Debug",                   // 1
    "NMI",                     // 2
    "Breakpoint",              // 3
    "Overflow",                // 4
    "BOUND Range Exceeded",    // 5
    "Invalid Opcode",          // 6
    "Device Not Available",    // 7
    "Double Fault",            // 8
    "Coprocessor Segment",     // 9
    "Invalid TSS",             // 10
    "Segment Not Present",     // 11
    "Stack-Segment Fault",     // 12
    "General Protection",      // 13
    "Page Fault",              // 14
    "Reserved",                // 15
    "x87 FP Exception",        // 16
    "Alignment Check",         // 17
    "Machine Check",           // 18
    "SIMD FP Exception",       // 19
    "Virtualization Exception",// 20
    "Control Protection",      // 21
    "Reserved",                // 22
    "Reserved",                // 23
    "Reserved",                // 24
    "Reserved",                // 25
    "Reserved",                // 26
    "Reserved",                // 27
    "Hypervisor Injection",    // 28 (if applicable)
    "VMM Communication",       // 29 (if applicable)
    "Security Exception",      // 30
    "Reserved"                 // 31
};

static void bsod_header(const char* title){
    console_set_color(CONSOLE_COLOR_WHITE, CONSOLE_COLOR_BLUE);
    console_clear();
    console_set_cursor(0, 0);
    console_puts("\n   *** RetaOS MAVI EKRAN ***\n\n");
    if (title && *title){
        console_puts("Hata: ");
        console_puts(title);
        console_puts("\n\n");
    }
}

static void bsod_regs(const struct isr_context* ctx){
    if (!ctx) return;
    console_puts("Kayitlar:\n");
    console_puts(" EAX="); console_put_hex(ctx->eax);
    console_puts(" ECX="); console_put_hex(ctx->ecx);
    console_puts(" EDX="); console_put_hex(ctx->edx);
    console_puts(" EBX="); console_put_hex(ctx->ebx);
    console_puts("\n ESP="); console_put_hex(ctx->esp);
    console_puts(" EBP="); console_put_hex(ctx->ebp);
    console_puts(" ESI="); console_put_hex(ctx->esi);
    console_puts(" EDI="); console_put_hex(ctx->edi);
    console_puts("\n\n");
}

// Genel amaçlı mavi ekran: başlık + mesaj + durdurma
void kernel_bsod(const char* msg, ...) {
    bsod_header("Kernel Hatasi");

    // Basit mesaj yazımı (formatlama olmadan)
    if (msg) {
        console_puts(msg);
    }
    console_puts("\n\nSistem durduruldu.\n");

    __asm__ __volatile__("cli");
    for (;;) { __asm__ __volatile__("hlt"); }
}

// Istisna icin detayli mavi ekran
void kernel_bsod_exception(uint32_t vector, uint32_t error_code, const struct isr_context* ctx, uint32_t cr2) {
    const char* name = (vector < (sizeof(s_exc_names)/sizeof(s_exc_names[0]))) ? s_exc_names[vector] : "Bilinmeyen Istisna";
    bsod_header("CPU Istisnasi");

    console_puts("Istisna: "); console_puts(name); console_puts(" (vector="); console_put_dec(vector); console_puts(")\n");
    console_puts("Hata Kodu: 0x"); console_put_hex(error_code); console_puts("  ("); console_put_dec(error_code); console_puts(")\n");
    if (vector == 14) { // Page Fault
        console_puts("CR2 (fault addr): 0x"); console_put_hex(cr2); console_puts("\n");
    }
    console_puts("\n");

    bsod_regs(ctx);

    console_puts("Yapilabilecekler:\n- Yeniden baslatin\n- Kernel loglarini kontrol edin (serial).\n");

    __asm__ __volatile__("cli");
    for (;;) { __asm__ __volatile__("hlt"); }
}
