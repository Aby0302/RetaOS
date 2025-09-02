#include <gui/gui.h>
#include <include/gui/display.h>
#include <kernel/console.h>
#include <kernel/kheap.h>
// String functions are already defined in kernel/string.c
// We'll use extern declarations instead
extern int strcmp(const char* s1, const char* s2);
extern char* strcpy(char* dest, const char* src);
extern size_t strlen(const char* str);
#include <stdint.h>
#include <stddef.h>

// Forward declarations
void gui_create_sample_windows(void);

// External dependencies
extern struct video_mode current_mode;

// Global GUI state
static desktop_t desktop;
static int gui_active = 0;

// VGA color conversion to RGB
static uint32_t vga_to_rgb(uint8_t vga_color) {
    static const uint32_t vga_palette[16] = {
        0x000000, // Black
        0x0000AA, // Blue
        0x00AA00, // Green
        0x00AAAA, // Cyan
        0xAA0000, // Red
        0xAA00AA, // Magenta
        0xAA5500, // Brown
        0xAAAAAA, // Light Grey
        0x555555, // Dark Grey
        0x5555FF, // Light Blue
        0x55FF55, // Light Green
        0x55FFFF, // Light Cyan
        0xFF5555, // Light Red
        0xFF55FF, // Light Magenta
        0xFFFF55, // Yellow
        0xFFFFFF  // White
    };
    return vga_palette[vga_color & 0x0F];
}

// Basic pixel drawing
static void gui_draw_pixel(int x, int y, uint8_t color) {
    if (x < 0 || y < 0 || x >= current_mode.width || y >= current_mode.height) {
        return;
    }

    uint32_t rgb_color = vga_to_rgb(color);
    display_set_pixel(x, y, rgb_color);
}

// Initialize GUI system
void gui_init(void) {
    // Add external serial function declaration
    extern void serial_write(const char* str);
    
    serial_write("[GUI] Initializing GUI system\r\n");
    
    // Initialize desktop
    desktop.bg_color = GUI_COLOR_BLUE;
    desktop.windows = NULL;
    desktop.focused_window = NULL;
    desktop.window_count = 0;
    desktop.next_window_id = 1;

    // GUI starts inactive - will be activated by ESC key
    gui_active = 0;
    
    serial_write("[GUI] GUI system initialized (inactive)\r\n");
}

// Show GUI
void gui_show(void) {
    // Add external serial function declaration
    extern void serial_write(const char* str);
    
    serial_write("[GUI] Activating GUI system\r\n");
    gui_active = 1;

    // Create sample windows for demonstration
    gui_create_sample_windows();

    gui_draw_desktop();
    serial_write("[GUI] GUI system activated\r\n");
}

// Hide GUI and return to console
void gui_hide(void) {
    // Add external serial function declaration
    extern void serial_write(const char* str);
    
    serial_write("[GUI] Deactivating GUI system\r\n");
    gui_active = 0;
    console_clear();
    serial_write("[GUI] GUI system deactivated\r\n");
}

// Clear screen with specified color
void gui_clear_screen(uint8_t color) {
    uint32_t rgb_color = vga_to_rgb(color);

    for (int y = 0; y < current_mode.height; y++) {
        for (int x = 0; x < current_mode.width; x++) {
            display_set_pixel(x, y, rgb_color);
        }
    }
}

// Draw desktop background
void gui_draw_desktop(void) {
    if (!gui_active) return;

    // Clear screen with desktop background color
    gui_clear_screen(desktop.bg_color);

    // Draw taskbar
    gui_draw_taskbar();

    // Draw all windows
    window_t* current = desktop.windows;
    while (current != NULL) {
        if (current->visible) {
            gui_draw_window(current);
        }
        current = current->next;
    }
}

// Draw taskbar at bottom of screen
void gui_draw_taskbar(void) {
    int taskbar_height = 30;
    int taskbar_y = current_mode.height - taskbar_height;

    // Draw taskbar background
    gui_draw_rect(0, taskbar_y, current_mode.width, taskbar_height, GUI_COLOR_LIGHT_GREY);

    // Draw taskbar border
    gui_draw_border(0, taskbar_y, current_mode.width, taskbar_height, GUI_COLOR_BLACK);

    // Draw start button
    gui_draw_button(5, taskbar_y + 5, 60, 20, "Start", GUI_COLOR_LIGHT_BLUE, GUI_COLOR_BLACK);

    // Draw window list area
    int window_list_x = 70;
    int window_list_width = current_mode.width - window_list_x - 100;

    if (window_list_width > 0) {
        gui_draw_rect(window_list_x, taskbar_y + 5, window_list_width, 20, GUI_COLOR_WHITE);
        gui_draw_border(window_list_x, taskbar_y + 5, window_list_width, 20, GUI_COLOR_DARK_GREY);
    }

    // Draw system tray
    int tray_x = current_mode.width - 90;
    gui_draw_rect(tray_x, taskbar_y + 5, 85, 20, GUI_COLOR_WHITE);
    gui_draw_border(tray_x, taskbar_y + 5, 85, 20, GUI_COLOR_DARK_GREY);
}

// Draw start menu (placeholder for now)
void gui_draw_start_menu(void) {
    // This will be implemented when we add start menu functionality
}

// Handle keyboard input
void gui_handle_keypress(char key) {
    if (!gui_active) return;

    // ESC key hides GUI and returns to console
    if (key == 27) { // ESC key
        gui_hide();
        return;
    }

    // Pass key to focused window if any
    if (desktop.focused_window) {
        // Window-specific key handling would go here
    }
}

// Update GUI (called periodically)
void gui_update(void) {
    if (!gui_active) return;

    // Redraw desktop if needed
    gui_draw_desktop();
}

// Draw a filled rectangle
void gui_draw_rect(int x, int y, int width, int height, uint8_t color) {
    for (int dy = 0; dy < height; dy++) {
        for (int dx = 0; dx < width; dx++) {
            gui_draw_pixel(x + dx, y + dy, color);
        }
    }
}

// Draw rectangle border
void gui_draw_border(int x, int y, int width, int height, uint8_t color) {
    // Top border
    for (int dx = 0; dx < width; dx++) {
        gui_draw_pixel(x + dx, y, color);
    }
    // Bottom border
    for (int dx = 0; dx < width; dx++) {
        gui_draw_pixel(x + dx, y + height - 1, color);
    }
    // Left border
    for (int dy = 0; dy < height; dy++) {
        gui_draw_pixel(x, y + dy, color);
    }
    // Right border
    for (int dy = 0; dy < height; dy++) {
        gui_draw_pixel(x + width - 1, y + dy, color);
    }
}

// Simple text drawing (basic implementation)
void gui_draw_text(int x, int y, const char* text, uint8_t color) {
    int current_x = x;
    int current_y = y;

    while (*text) {
        if (*text == '\n') {
            current_x = x;
            current_y += 12; // Basic line height
        } else {
            // Draw character (simplified - just a placeholder)
            // In a real implementation, you'd use a font bitmap
            gui_draw_pixel(current_x, current_y, color);
            gui_draw_pixel(current_x + 1, current_y, color);
            gui_draw_pixel(current_x, current_y + 1, color);
            gui_draw_pixel(current_x + 1, current_y + 1, color);
            current_x += 8; // Basic character width
        }
        text++;
    }
}

// Draw a button
void gui_draw_button(int x, int y, int width, int height, const char* text, uint8_t bg_color, uint8_t fg_color) {
    // Draw button background
    gui_draw_rect(x, y, width, height, bg_color);

    // Draw button border
    gui_draw_border(x, y, width, height, GUI_COLOR_BLACK);

    // Draw button text (centered)
    int text_x = x + (width - (strlen(text) * 8)) / 2;
    int text_y = y + (height - 12) / 2;
    gui_draw_text(text_x, text_y, text, fg_color);
}

// Set cursor position (placeholder)
void gui_set_cursor_pos(int x, int y) {
    // In a real implementation, this would update hardware cursor
}

// Check if GUI is active
int gui_is_active(void) {
    return gui_active;
}

// Window management functions
window_t* gui_create_window(int x, int y, int width, int height, const char* title) {
    window_t* window = (window_t*)kmalloc(sizeof(window_t));
    if (!window) return NULL;

    window->id = desktop.next_window_id++;
    window->x = x;
    window->y = y;
    window->width = width;
    window->height = height;
    strcpy(window->title, title);
    window->bg_color = GUI_COLOR_WHITE;
    window->fg_color = GUI_COLOR_BLACK;
    window->border_color = GUI_COLOR_BLACK;
    window->state = WINDOW_STATE_NORMAL;
    window->visible = 1;
    window->focused = 0;
    window->next = desktop.windows;

    desktop.windows = window;
    desktop.window_count++;

    return window;
}

void gui_destroy_window(window_t* window) {
    if (!window) return;

    // Remove from linked list
    if (desktop.windows == window) {
        desktop.windows = window->next;
    } else {
        window_t* current = desktop.windows;
        while (current && current->next != window) {
            current = current->next;
        }
        if (current) {
            current->next = window->next;
        }
    }

    // Update focused window if necessary
    if (desktop.focused_window == window) {
        desktop.focused_window = NULL;
    }

    desktop.window_count--;
    kfree(window);
}

void gui_draw_window(window_t* window) {
    if (!window || !window->visible) return;

    // Draw window background
    gui_draw_rect(window->x, window->y, window->width, window->height, window->bg_color);

    // Draw window border
    gui_draw_border(window->x, window->y, window->width, window->height, window->border_color);

    // Draw title bar
    int title_bar_height = 25;
    gui_draw_rect(window->x, window->y, window->width, title_bar_height, GUI_COLOR_LIGHT_BLUE);

    // Draw title text
    gui_draw_text(window->x + 5, window->y + 5, window->title, GUI_COLOR_BLACK);

    // Draw close button
    gui_draw_button(window->x + window->width - 25, window->y + 2, 20, 20, "X", GUI_COLOR_LIGHT_RED, GUI_COLOR_WHITE);
}

void gui_focus_window(window_t* window) {
    if (!window) return;

    // Unfocus current window
    if (desktop.focused_window) {
        desktop.focused_window->focused = 0;
    }

    // Focus new window
    desktop.focused_window = window;
    window->focused = 1;

    // Move to front (simplified - just redraw)
    gui_draw_desktop();
}

void gui_move_window(window_t* window, int new_x, int new_y) {
    if (!window) return;

    window->x = new_x;
    window->y = new_y;
    gui_draw_desktop();
}

void gui_resize_window(window_t* window, int new_width, int new_height) {
    if (!window) return;

    window->width = new_width;
    window->height = new_height;
    gui_draw_desktop();
}

// Create sample windows for demonstration
void gui_create_sample_windows(void) {
    // Clear any existing windows first
    window_t* current = desktop.windows;
    while (current != NULL) {
        window_t* next = current->next;
        kfree(current);
        current = next;
    }
    desktop.windows = NULL;
    desktop.focused_window = NULL;
    desktop.window_count = 0;
    desktop.next_window_id = 1;

    // Create sample windows
    window_t* window1 = gui_create_window(50, 50, 300, 200, "RetaOS Terminal");
    if (window1) {
        window1->bg_color = GUI_COLOR_BLACK;
        window1->fg_color = GUI_COLOR_LIGHT_GREEN;
    }

    window_t* window2 = gui_create_window(400, 100, 250, 150, "File Manager");
    if (window2) {
        window2->bg_color = GUI_COLOR_WHITE;
        window2->fg_color = GUI_COLOR_BLACK;
    }

    window_t* window3 = gui_create_window(150, 300, 350, 180, "System Information");
    if (window3) {
        window3->bg_color = GUI_COLOR_LIGHT_GREY;
        window3->fg_color = GUI_COLOR_BLACK;
    }

    // Focus the first window
    if (window1) {
        gui_focus_window(window1);
    }
}