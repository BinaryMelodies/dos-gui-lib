#ifndef __INTERNAL_H
#define __INTERNAL_H

/*** Internal references that are included in all backends ***/

#ifndef _COMPILE_LIBRARY
# error should only be included during library compilation
#endif
#undef _COMPILE_LIBRARY

#include "api.h"

extern gui_callback_show_t far * callback_show;
extern gui_callback_key_t far * callback_key_press;
extern gui_callback_key_t far * callback_key_release;
extern gui_callback_text_t far * callback_text;
extern gui_callback_mouse_button_t far * callback_mouse_button_press;
extern gui_callback_mouse_button_t far * callback_mouse_button_release;
extern gui_callback_mouse_move_t far * callback_mouse_move;
extern gui_callback_quit_t far * callback_quit;

extern GuiMouseButton_t callback_mouse_buttons_mask;
extern GuiMouseButton_t callback_click_count_mask;

#endif // __INTERNAL_H
