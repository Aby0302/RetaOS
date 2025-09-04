#include "include/kernel/splash.h"
#include <stdint.h>
#include "include/kernel/debug.h"
#include "include/arch/x86/io.h"
#include "include/drivers/keyboard.h"
#include "include/drivers/serial.h"
#include "include/kernel/timer.h"
#define __FILENAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)

// External function declaration
extern void kprintf(const char* fmt, ...);

#define VGA_TEXT_WIDTH 80
#define VGA_TEXT_HEIGHT 25
#define VGA_TEXT_ADDR 0xB8000

// Forward declarations
static void vga_text_putstr(int x, int y, const char* str, uint8_t color);

// Tuş bekleme fonksiyonu
// Not: IRQ sürücüsü tamponundan okur; seri konsol için CR/LF de kabul edilir.
// Zaman aşımı için kaba bir spin kullandık; timer_ticks kullanılmıyor.
static void wait_for_keypress(void) {
    vga_text_putstr(20, 24, "Waiting for key press... Press ENTER!", 0x0F);

    // Yaklaşık ~1-2 saniye kadar bekle (ortam hızına göre değişir)
    for (volatile uint32_t spin = 0; spin < 10000000; ++spin) {
        int ch = keyboard_getchar_nonblock();
        if (ch == '\n') break;
        int s = serial_getchar_nonblock();
        if (s == '\r' || s == '\n') break;
        __asm__ __volatile__("pause");
        if (spin == 9999999) {
            vga_text_putstr(20, 24, "No keypress; auto-continue...        ", 0x0E);
        }
    }

    vga_text_putstr(20, 24, "Continuing boot...                     ", 0x0A);
    for (volatile int i = 0; i < 1000000; ++i) __asm__ __volatile__("pause");
}

// VGA text buffer'a yaz
static void vga_text_putchar(int x, int y, char c, uint8_t color) {
    if (x >= 0 && x < VGA_TEXT_WIDTH && y >= 0 && y < VGA_TEXT_HEIGHT) {
        uint16_t* vga = (uint16_t*)VGA_TEXT_ADDR;
        vga[y * VGA_TEXT_WIDTH + x] = (color << 8) | c;
    }
}

// VGA text buffer'a string yaz
static void vga_text_putstr(int x, int y, const char* str, uint8_t color) {
    int i = 0;
    while (str[i] && x + i < VGA_TEXT_WIDTH) {
        vga_text_putchar(x + i, y, str[i], color);
        i++;
    }
}

// VGA text buffer'ı temizle
static void vga_text_clear(uint8_t color) {
    uint16_t* vga = (uint16_t*)VGA_TEXT_ADDR;
    for (int i = 0; i < VGA_TEXT_WIDTH * VGA_TEXT_HEIGHT; i++) {
        vga[i] = (color << 8) | ' ';
    }
}

void splash_show(void) {
    // Debug mesajı
    debug_print(DEBUG_LEVEL_INFO, __FILENAME__, __LINE__, __func__, "=== Starting splash screen initialization ===");
    
    // VGA text buffer'ı temizle
    vga_text_clear(0x0F); // Beyaz yazı, siyah arka plan
    
    // Büyük RETAOS logosu
    vga_text_putstr(25, 2, "=================================================", 0x0F); // Beyaz
    vga_text_putstr(25, 3, "                RETAOS v1.0", 0x0E); // Sarı
    vga_text_putstr(25, 4, "=================================================", 0x0F); // Beyaz
    vga_text_putstr(25, 5, "", 0x0F);
    
    // ASCII Art Logo
    vga_text_putstr(20, 7, "RRRRR   EEEEE  TTTTT   AAAAA   OOOOO00   SSSSSSS", 0x0C); // Kırmızı
    vga_text_putstr(20, 8, "R   R   E        T     A   A   O     O  S", 0x0A); // Yeşil
    vga_text_putstr(20, 9, "RRRRR   EEEE     T     AAAAA   O     O  SSSSS", 0x0B); // Cyan
    vga_text_putstr(20, 10, "R   R   E        T     A   A   O     O      S", 0x0D); // Magenta
    vga_text_putstr(20, 11, "R   R   EEEEE    T     A   A    OOOOO  SSSSS", 0x0E); // Sarı
    
    vga_text_putstr(25, 13, "", 0x0F);
    vga_text_putstr(25, 14, "        Operating System Kernel", 0x0B); // Cyan
    vga_text_putstr(25, 15, "        Initializing...", 0x0A); // Yeşil
    vga_text_putstr(25, 16, "", 0x0F);
    
    // Initial progress bar setup
    vga_text_putstr(20, 18, "Progress: [", 0x0F); // Beyaz
    vga_text_putstr(65, 18, "] 0%", 0x0F); // Beyaz
    
    debug_print(DEBUG_LEVEL_DEBUG, __FILENAME__, __LINE__, __func__, "Splash screen displayed successfully");
}

void splash_update_progress(int percent) {
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    
    // Progress bar'ı güncelle
    int filled = (percent * 30) / 100;
    for (int i = 0; i < 30; i++) {
        if (i < filled) {
            vga_text_putchar(35 + i, 18, '#', 0x0A); // Yeşil dolgu
        } else {
            vga_text_putchar(35 + i, 18, ' ', 0x08); // Gri arka plan
        }
    }
    
    // Yüzdeyi güncelle
    char percent_str[10];
    // Basit int to string dönüşümü
    int temp = percent;
    int len = 0;
    if (temp == 0) {
        percent_str[0] = '0';
        len = 1;
    } else {
        while (temp > 0) {
            percent_str[len] = '0' + (temp % 10);
            temp /= 10;
            len++;
        }
    }
    // Ters çevir
    for (int i = 0; i < len / 2; i++) {
        char t = percent_str[i];
        percent_str[i] = percent_str[len - 1 - i];
        percent_str[len - 1 - i] = t;
    }
    percent_str[len] = '%';
    percent_str[len + 1] = '\0';
    
    vga_text_putstr(65, 18, percent_str, 0x0F);
}

void splash_hide(void) {
    // Splash screen'i gizle - ekranı temizle
    vga_text_clear(0x07); // Siyah arka plan, beyaz yazı
}

// Kernel will provide this to continue boot (start shell/scheduler)
extern void kernel_continue_after_splash(void);

void splash_show_complete(void) {
    // Show completion message
    vga_text_putstr(20, 20, "System initialized successfully!", 0x0E); // Sarı
    vga_text_putstr(20, 21, "RetaOS is now running!", 0x0A); // Yeşil
    vga_text_putstr(20, 22, "Press ENTER to toggle GUI mode", 0x0B); // Cyan
    vga_text_putstr(20, 23, "Press ENTER to continue...", 0x0F); // Beyaz
    
    debug_print(DEBUG_LEVEL_INFO, __FILENAME__, __LINE__, __func__, "Boot completion message displayed");
    
    // Do not block here; in serial/graphics-less boots ENTER may never arrive.
    // We already show a message above; proceed automatically to user-mode init.
    // If you want to re-enable key wait, call wait_for_keypress() here.
    wait_for_keypress();
    //for (volatile int i = 0; i < 200000; ++i) __asm__ __volatile__("pause");

    splash_hide();          // ekranı temizle
    // GUI yolunu daha sonra etkinleştireceğiz. Şimdilik metin konsolunda
    // kal ve hiçbir grafik framebuffer işlemi yapma ki shell görünsün.
    
    debug_print(DEBUG_LEVEL_INFO, __FILENAME__, __LINE__, __func__, "User pressed key, continuing to scheduler");
    // Hand control back to kernel to start shell/scheduler
    kernel_continue_after_splash();
}
