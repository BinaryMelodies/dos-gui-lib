
#include "api.h"

gui_callback_show_t far * callback_show = NULL;
gui_callback_key_t far * callback_keypress = NULL;
gui_callback_key_t far * callback_keyrelease = NULL;
gui_callback_text_t far * callback_text = NULL;
gui_callback_button_t far * callback_buttonpress = NULL;
gui_callback_button_t far * callback_buttonrelease = NULL;
gui_callback_mouse_t far * callback_mousemove = NULL;
gui_callback_quit_t far * callback_quit = NULL;

GuiMouseButton_t callback_buttons_mask = 0;
GuiMouseButton_t callback_clickcount_mask = 0;

void gui_register_callback_show(gui_callback_show_t far * show)
{
	callback_show = show;
}

void gui_register_callback_keypress(gui_callback_key_t far * keypress)
{
	callback_keypress = keypress;
}

void gui_register_callback_keyrelease(gui_callback_key_t far * keyrelease)
{
	callback_keyrelease = keyrelease;
}

void gui_register_callback_text(gui_callback_text_t far * text)
{
	callback_text = text;
}

void gui_observe_buttons(GuiMouseButton_t buttons, GuiMouseButton_t click_count_mask)
{
	callback_buttons_mask = buttons;
	callback_clickcount_mask = click_count_mask;
}

void gui_register_callback_buttonpress(gui_callback_button_t far * buttonpress)
{
	callback_buttonpress = buttonpress;
}

void gui_register_callback_buttonrelease(gui_callback_button_t far * buttonrelease)
{
	callback_buttonrelease = buttonrelease;
}

void gui_register_callback_mousemove(gui_callback_mouse_t far * mousemove)
{
	callback_mousemove = mousemove;
}

void gui_register_callback_quit(gui_callback_quit_t far * quit)
{
	callback_quit = quit;
}

