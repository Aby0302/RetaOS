#include "include/kernel/splash.h"
#include <stdint.h>
#include "include/kernel/debug.h"
#include "include/arch/x86/io.h"
#include "include/drivers/keyboard.h"
#define __FILENAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)

// External function declaration
extern void kprintf(const char* fmt, ...);

#define VGA_TEXT_WIDTH 80
#define VGA_TEXT_HEIGHT 25
#define VGA_TEXT_ADDR 0xB8000

// Forward declarations
static void vga_text_putstr(int x, int y, const char* str, uint8_t color);

// Tuş bekleme fonksiyonu
static void wait_for_keypress(void) {
    // Klavye tamponunu boşalt
    while (inb(0x64) & 1) (void)inb(0x60);

    vga_text_putstr(20, 24, "Waiting for key press... Press ENTER!", 0x0F);

    int e0 = 0;
    for (;;) {
        uint8_t st = inb(0x64);
        if (st & 1) {                    // Output buffer dolu
            uint8_t sc = inb(0x60);      // scancode set 1
            if (sc == 0xE0) { e0 = 1; continue; } // keypad enter için ön ek
            // make-code (basma anı), break değil
            if ((sc & 0x80) == 0) {
                uint8_t code = sc & 0x7F;
                if (code == 0x1C) break;         // "Enter" (ana klavye)
                if (e0 && code == 0x1C) break;   // "Keypad Enter"
            }
            e0 = 0;
        }
        // Kısa gecikme (IRQ gerektirmez)
        for (volatile int i = 0; i < 20000; ++i) __asm__ __volatile__("pause");
    }

    vga_text_putstr(20, 24, "Key pressed! Starting system...       ", 0x0A);
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

void splash_show_complete(void) {
    // Show completion message
    vga_text_putstr(20, 20, "System initialized successfully!", 0x0E); // Sarı
    vga_text_putstr(20, 21, "RetaOS is now running!", 0x0A); // Yeşil
    vga_text_putstr(20, 22, "Press ENTER to toggle GUI mode", 0x0B); // Cyan
    vga_text_putstr(20, 23, "Press any key to continue...", 0x0F); // Beyaz
    
    debug_print(DEBUG_LEVEL_INFO, __FILENAME__, __LINE__, __func__, "Boot completion message displayed");
    
    // Wait for ENTER key press
    wait_for_keypress();

    splash_hide();          // ekranı temizle
    debug_print(DEBUG_LEVEL_INFO,__FILENAME__,__LINE__,__func__,"GUI init...");
    //gui_init();             // <-- kendi GUI başlatıcını çağır
    // DUMAN TESTİ: mutlaka hemen boyayın
    debug_print(DEBUG_LEVEL_INFO,__FILENAME__,__LINE__,__func__,"GUI initialized");
    // DUMAN TESTİ: mutlaka hemen boyayın
    //extern void display_clear(uint32_t rgb);
    //display_clear(0x334455); // tüm ekran renklenmeli    
    //gui_show();             // GUIyi göster
    //console_init();
    //console_show();
    debug_print(DEBUG_LEVEL_INFO,__FILENAME__,__LINE__,__func__,"GUI shown");

    debug_print(DEBUG_LEVEL_INFO, __FILENAME__, __LINE__, __func__, "User pressed key, continuing to scheduler");
}
