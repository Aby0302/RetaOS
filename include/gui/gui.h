#ifndef GUI_H
#define GUI_H

#include <stdint.h>
#include <stddef.h>

// GUI Colors (VGA 16-color palette)
#define GUI_COLOR_BLACK         0x00
#define GUI_COLOR_BLUE          0x01
#define GUI_COLOR_GREEN         0x02
#define GUI_COLOR_CYAN          0x03
#define GUI_COLOR_RED           0x04
#define GUI_COLOR_MAGENTA       0x05
#define GUI_COLOR_BROWN         0x06
#define GUI_COLOR_LIGHT_GREY    0x07
#define GUI_COLOR_DARK_GREY     0x08
#define GUI_COLOR_LIGHT_BLUE    0x09
#define GUI_COLOR_LIGHT_GREEN   0x0A
#define GUI_COLOR_LIGHT_CYAN    0x0B
#define GUI_COLOR_LIGHT_RED     0x0C
#define GUI_COLOR_LIGHT_MAGENTA 0x0D
#define GUI_COLOR_YELLOW        0x0E
#define GUI_COLOR_WHITE         0x0F

// Window states
#define WINDOW_STATE_NORMAL     0
#define WINDOW_STATE_MINIMIZED  1
#define WINDOW_STATE_MAXIMIZED  2

// GUI Events
#define GUI_EVENT_NONE          0
#define GUI_EVENT_KEY_PRESS     1
#define GUI_EVENT_MOUSE_CLICK   2
#define GUI_EVENT_WINDOW_CLOSE  3
#define GUI_EVENT_WINDOW_MOVE   4

// Window structure
typedef struct window {
    int id;
    int x, y;
    int width, height;
    char title[64];
    uint8_t bg_color;
    uint8_t fg_color;
    uint8_t border_color;
    int state;
    int visible;
    int focused;
    struct window* next;
} window_t;

// Desktop structure
typedef struct {
    uint8_t bg_color;
    window_t* windows;
    window_t* focused_window;
    int window_count;
    int next_window_id;
} desktop_t;

// GUI Event structure
typedef struct {
    int type;
    int data1;
    int data2;
    char key;
} gui_event_t;

// GUI Functions
void gui_init(void);
void gui_show(void);
void gui_hide(void);
void gui_clear_screen(uint8_t color);
void gui_draw_desktop(void);
void gui_draw_taskbar(void);
void gui_draw_start_menu(void);
void gui_handle_keypress(char key);
void gui_update(void);

// Window management
window_t* gui_create_window(int x, int y, int width, int height, const char* title);
void gui_destroy_window(window_t* window);
void gui_draw_window(window_t* window);
void gui_focus_window(window_t* window);
void gui_move_window(window_t* window, int new_x, int new_y);
void gui_resize_window(window_t* window, int new_width, int new_height);

// Drawing functions
void gui_draw_rect(int x, int y, int width, int height, uint8_t color);
void gui_draw_border(int x, int y, int width, int height, uint8_t color);
void gui_draw_text(int x, int y, const char* text, uint8_t color);
void gui_draw_button(int x, int y, int width, int height, const char* text, uint8_t bg_color, uint8_t fg_color);

// Utility functions
void gui_set_cursor_pos(int x, int y);
int gui_is_active(void);

#endif // GUI_H