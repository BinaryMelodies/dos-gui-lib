#ifndef __API_H
#define __API_H

#include "types.h"

/* This must come before stddef.h */
#include "pfdefs.h"

#include <stddef.h>
#include "libc.h"

/* Toolkit management */
void gui_init(GuiMainParameters_t * parameters);
void gui_terminate(void);
int gui_main_loop(void);
void gui_terminate_main_loop(void);
int gui_message_box(const char * title, const char * message, int buttons, int default_button, GuiMessageBoxIcon_t icon);

/* Window management */
GuiWindow_t gui_window_create(const char * window_title, int x, int y, int w, int h);
void gui_window_destroy(GuiWindow_t window);
GuiRectangle_t gui_window_get_client_area(GuiWindow_t window);
void gui_window_redraw(GuiWindow_t window);
GuiDrawContext_t gui_window_begin_draw(GuiWindow_t window);
void gui_window_end_draw(GuiDrawContext_t draw_context);

/* Drawing tools */
void gui_set_color_black(GuiDrawContext_t draw_context);
void gui_set_color_white(GuiDrawContext_t draw_context);
void gui_fill_rectangle(GuiDrawContext_t draw_context, int x, int y, int w, int h);
void gui_draw_line(GuiDrawContext_t draw_context, int x1, int y1, int x2, int y2);
int gui_get_font_height(GuiDrawContext_t draw_context);
void gui_write_text(GuiDrawContext_t draw_context, int x, int y, const char * text);

/* Event management */
GuiKey_t gui_get_keycode(GuiKeyEvent_t event);
GuiPoint_t gui_get_button_coordinates(GuiButtonEvent_t event);
GuiMouseButton_t gui_get_buttons(GuiButtonEvent_t event);
GuiMouseButton_t gui_is_double_click(GuiButtonEvent_t event);

GuiPoint_t gui_get_mouse_coordinates(GuiMouseEvent_t event);

typedef bool gui_callback_show_t(GuiWindow_t window);
void gui_register_callback_show(gui_callback_show_t far * show);

typedef bool gui_callback_key_t(GuiWindow_t window, GuiKeyEvent_t key);
void gui_register_callback_keypress(gui_callback_key_t far * keypress);
void gui_register_callback_keyrelease(gui_callback_key_t far * keypress);

typedef bool gui_callback_text_t(GuiWindow_t window, size_t count, char far * text);
void gui_register_callback_text(gui_callback_text_t far * text);

typedef bool gui_callback_button_t(GuiWindow_t window, GuiButtonEvent_t button);
void gui_observe_buttons(GuiMouseButton_t buttons, GuiMouseButton_t click_count_mask);
void gui_register_callback_buttonpress(gui_callback_button_t far * buttonpress);
void gui_register_callback_buttonrelease(gui_callback_button_t far * buttonrelease);

typedef bool gui_callback_mouse_t(GuiWindow_t window, GuiMouseEvent_t mousemove);
void gui_register_callback_mousemove(gui_callback_mouse_t far * buttonpress);

typedef bool gui_callback_quit_t(GuiWindow_t window);
void gui_register_callback_quit(gui_callback_quit_t far * quit);

/* Main entry point, must be defined */
extern int gui_main(GuiMainParameters_t parameters);

#endif // __API_H
