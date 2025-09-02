#ifndef SPLASH_H
#define SPLASH_H

#include <stdint.h>

// GUI functions - declared here to prevent implicit function declaration errors
extern void gui_init(void);
extern void gui_show(void);

void splash_show(void);
void splash_update_progress(int percent);
void splash_hide(void);
void splash_show_complete(void);

#endif // SPLASH_H
