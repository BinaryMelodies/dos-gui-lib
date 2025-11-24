#ifndef __INTERNAL_H
#define __INTERNAL_H

// TODO: should only be included during library compilation

#include "api.h"

extern gui_callback_show_t far * callback_show;
extern gui_callback_key_t far * callback_keypress;
extern gui_callback_key_t far * callback_keyrelease;
extern gui_callback_text_t far * callback_text;
extern gui_callback_button_t far * callback_buttonpress;
extern gui_callback_button_t far * callback_buttonrelease;
extern gui_callback_mouse_t far * callback_mousemove;
extern gui_callback_quit_t far * callback_quit;

extern GuiMouseButton_t callback_buttons_mask;
extern GuiMouseButton_t callback_clickcount_mask;

#endif // __INTERNAL_H
