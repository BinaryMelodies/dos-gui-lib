#ifndef __API_H
#define __API_H

/*** The main API of the library ***/

#include "types.h"

/* This must come before stddef.h */
#include "pfdefs.h"

#include <stddef.h>
#include "libc.h"

/* * * Toolkit management * * */
/** Must be called before any other function call **/
void gui_init(GuiMainParameters_t * parameters);
/** Should be called when the library is no longer needed **/
void gui_terminate(void);
/** Handles GUI events and passes them over to callbacks **/
int gui_main_loop(void);
/** Informs the GUI main loop that execution should be terminated **/
void gui_terminate_main_loop(void);
/** Displays a modal dialog **/
int gui_message_box(const char * title, const char * message, GuiMessageBoxButtonSet_t buttons, int default_button, GuiMessageBoxIcon_t icon);

/* * * Window management * * */
/** Creates and displays a window with the specified title, coordinates and dimensions **/
GuiWindow_t gui_window_create(const char * window_title, int x, int y, int w, int h);
/** Tells the GUI that the window can be disposed of **/
void gui_window_destroy(GuiWindow_t window);
/** Retrieves the dimensions of the area of the window that belongs to the application **/
GuiRectangle_t gui_window_get_client_area(GuiWindow_t window);
/** Instructs the GUI to make the window redraw its contents, window drawing routines should only appear in the redraw callback **/
void gui_window_redraw(GuiWindow_t window);
/** Informs the GUI that the window is currently being drawn to, to be called within the redraw callback **/
GuiDrawContext_t gui_window_begin_draw(GuiWindow_t window);
/** Informs the GUI that the window has finished being drawn to, to be called within the redraw callback **/
void gui_window_end_draw(GuiDrawContext_t * draw_context);

/* * * Drawing tools * * */
/** Sets the current drawing color to black **/
void gui_set_color_black(GuiDrawContext_t * draw_context);
/** Sets the current drawing color to white **/
void gui_set_color_white(GuiDrawContext_t * draw_context);
/** Fills a rectangle whose borders are parallel to the axes with the current color **/
void gui_fill_rectangle(GuiDrawContext_t * draw_context, int x, int y, int w, int h);
/** Draws a line segment **/
void gui_draw_line(GuiDrawContext_t * draw_context, int x1, int y1, int x2, int y2);
/** Fetches the full height of the current font, which can be used to calculate the distance between lines **/
int gui_get_font_height(GuiDrawContext_t * draw_context);
/** Displays text using the current font, at the upper left corner of the text provided **/
void gui_write_text(GuiDrawContext_t * draw_context, int x, int y, const char * text);

/* * * Event management * * */
/** Retrieves the key code associated with the key event **/
GuiKey_t gui_get_keycode(GuiKeyEvent_t event);
/** Retrieves the coordinates of the cursor at the moment this event was issued **/
GuiPoint_t gui_get_mouse_button_coordinates(GuiMouseButtonEvent_t event);
/** Retrieves the mouse button that has been pressed for this event **/
GuiMouseButton_t gui_get_mouse_buttons(GuiMouseButtonEvent_t event);
/** Retrieves the bit flag determining whether the mouse button was clicked once or twice **/
GuiMouseButton_t gui_is_double_click(GuiMouseButtonEvent_t event);
/** Retrieves the coordinates of the cursor at the moment this event was issued **/
GuiPoint_t gui_get_mouse_move_coordinates(GuiMouseMoveEvent_t event);

/* * * Callbacks * * */
typedef bool gui_callback_show_t(GuiWindow_t window);
/** Callback for displaying the window contents, must be registered before creating any windows **/
void gui_register_callback_show(gui_callback_show_t far * show);

/** Used for key presses and releases, with the details of the event passed as arguments **/
typedef bool gui_callback_key_t(GuiWindow_t window, GuiKeyEvent_t key_event);
/** Callback when a key is pressed, must be registered before creating any windows **/
void gui_register_callback_key_press(gui_callback_key_t far * key_press);
/** Callback when a key is released (note that the backend might not support this), must be registered before creating any windows **/
void gui_register_callback_key_release(gui_callback_key_t far * key_release);

/** Used for text input, with a sequence of characters. This is typically called alongside a keypress event with a single character **/
typedef bool gui_callback_text_t(GuiWindow_t window, size_t count, char far * text);
/** Callback when text is typed, must be registered before creating any windows **/
void gui_register_callback_text(gui_callback_text_t far * text);

typedef bool gui_callback_mouse_button_t(GuiWindow_t window, GuiMouseButtonEvent_t mouse_button_event);
/** Declares what type of clicks will be observed (note that the backend might not support all combinations), must be called before creating any windows **/
void gui_observe_mouse_buttons(GuiMouseButton_t mouse_buttons, GuiMouseButton_t click_count_mask);
/** Callback when a mouse button is pressed (including possibly double clicks), must be registered before creating any windows **/
void gui_register_callback_mouse_button_press(gui_callback_mouse_button_t far * mouse_button_press);
/** Callback when a mouse button is released, must be registered before creating any windows **/
void gui_register_callback_mouse_button_release(gui_callback_mouse_button_t far * mouse_button_release);

typedef bool gui_callback_mouse_move_t(GuiWindow_t window, GuiMouseMoveEvent_t mouse_move);
/** Callback when the mouse cursor is moved, must be registered before creating any windows **/
void gui_register_callback_mouse_move(gui_callback_mouse_move_t far * mouse_move);

typedef bool gui_callback_quit_t(GuiWindow_t window);
/** Callback when the window is closed by the user, must be registered before creating any windows **/
void gui_register_callback_quit(gui_callback_quit_t far * quit);

/* * * Graphical widgets * * */
/* Creates a push button */
GuiWidget_t gui_create_push_button(GuiWindow_t window, GuiWidget_t parent, int x, int y, int w, int h, const char far * caption, long flags);
#define GuiWindowRoot ((GuiWidget_t) 0)

typedef void gui_callback_action_t(GuiWindow_t window, GuiWidget_t widget, int parameter);
/** Callback for responding to behavior from interacting with the widget **/
void gui_register_callback_action(gui_callback_action_t far * action);

/* * * Main entry point, must be defined * * */
extern int gui_main(GuiMainParameters_t parameters);

#endif // __API_H
