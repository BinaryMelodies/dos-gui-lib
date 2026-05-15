
/*** Implementation that is shared by all backends ***/

#include "api.h"

gui_callback_show_t far * callback_show = NULL;
gui_callback_key_t far * callback_key_press = NULL;
gui_callback_key_t far * callback_key_release = NULL;
gui_callback_text_t far * callback_text = NULL;
gui_callback_mouse_button_t far * callback_mouse_button_press = NULL;
gui_callback_mouse_button_t far * callback_mouse_button_release = NULL;
gui_callback_mouse_move_t far * callback_mouse_move = NULL;
gui_callback_quit_t far * callback_quit = NULL;
gui_callback_action_t far * callback_action = NULL;

GuiMouseButton_t callback_mouse_buttons_mask = 0;
GuiMouseButton_t callback_click_count_mask = 0;

void gui_register_callback_show(gui_callback_show_t far * show)
{
	callback_show = show;
}

void gui_register_callback_key_press(gui_callback_key_t far * key_press)
{
	callback_key_press = key_press;
}

void gui_register_callback_keyrelease(gui_callback_key_t far * key_release)
{
	callback_key_release = key_release;
}

void gui_register_callback_text(gui_callback_text_t far * text)
{
	callback_text = text;
}

void gui_observe_mouse_buttons(GuiMouseButton_t mouse_buttons, GuiMouseButton_t click_count_mask)
{
	callback_mouse_buttons_mask = mouse_buttons;
	callback_click_count_mask = click_count_mask;
}

void gui_register_callback_mouse_button_press(gui_callback_mouse_button_t far * mouse_button_press)
{
	callback_mouse_button_press = mouse_button_press;
}

void gui_register_callback_mouse_button_release(gui_callback_mouse_button_t far * mouse_button_release)
{
	callback_mouse_button_release = mouse_button_release;
}

void gui_register_callback_mouse_move(gui_callback_mouse_move_t far * mouse_move)
{
	callback_mouse_move = mouse_move;
}

void gui_register_callback_quit(gui_callback_quit_t far * quit)
{
	callback_quit = quit;
}

void gui_register_callback_action(gui_callback_action_t far * action)
{
	callback_action = action;
}

