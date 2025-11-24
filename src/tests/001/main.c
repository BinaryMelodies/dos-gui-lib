
#include "api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libc.h"

#if GUI_WIN
# define GUINAME "Windows"
#elif GUI_GEM
# define GUINAME "GEM"
#elif GUI_DVX
# define GUINAME "DESQview/X"
#elif GUI_OS2
# define GUINAME "OS/2"
#endif

#define WINDOW_TITLE "Test in " GUINAME

char message_buffer[256] = "Brave new world!";

static bool far _callback_show(GuiWindow_t window)
{
	GuiDrawContext_t draw_context = gui_window_begin_draw(window);
	GuiRectangle_t client_area = gui_window_get_client_area(window);

	gui_set_color_black(draw_context);
	gui_fill_rectangle(draw_context, 0, 0, client_area.w, client_area.h);
	gui_set_color_white(draw_context);
	gui_draw_line(draw_context, 10, 10, 30, 30);
	gui_write_text(draw_context, 10, 20, message_buffer);
	gui_window_end_draw(draw_context);

	return true;
}

static int keypress_counter = 0;

static const char far * key_names[] =
{
	[KeyBackspace] = "Backspace",
	[KeyTab] = "Tab",
	[KeyEnter] = "Enter",
	[KeyEscape] = "Escape",
	[' '] = "Space",
	['\''] = "'",
	[KeyAsterisk] = "Numeric '*'",
	[KeyPlus] = "Numeric '+'",
	[','] = ",",
	['-'] = "-",
	['.'] = ".",
	['/'] = "/",
	['0'] = "0",
	['1'] = "1",
	['2'] = "2",
	['3'] = "3",
	['4'] = "4",
	['5'] = "5",
	['6'] = "6",
	['7'] = "7",
	['8'] = "8",
	['9'] = "9",
	[';'] = ";",
	['='] = "=",
	['['] = "[",
	['\\'] = "\\",
	[']'] = "]",
	['`'] = "`",
	['a'] = "A",
	['b'] = "B",
	['c'] = "C",
	['d'] = "D",
	['e'] = "E",
	['f'] = "F",
	['g'] = "G",
	['h'] = "H",
	['i'] = "I",
	['j'] = "J",
	['k'] = "K",
	['l'] = "L",
	['m'] = "M",
	['n'] = "N",
	['o'] = "O",
	['p'] = "P",
	['q'] = "Q",
	['r'] = "R",
	['s'] = "S",
	['t'] = "T",
	['u'] = "U",
	['v'] = "V",
	['w'] = "W",
	['x'] = "X",
	['y'] = "Y",
	['z'] = "Z",
	[KeyLeft] = "Left",
	[KeyRight] = "Right",
	[KeyUp] = "Up",
	[KeyDown] = "Down",
	[KeyPageUp] = "Page Up",
	[KeyPageDown] = "Page Down",
	[KeyHome] = "Home",
	[KeyEnd] = "End",
	[KeyInsert] = "Insert",
	[KeyDelete] = "Delete",
	[KeyF1] = "F1",
	[KeyF2] = "F2",
	[KeyF3] = "F3",
	[KeyF4] = "F4",
	[KeyF5] = "F5",
	[KeyF6] = "F6",
	[KeyF7] = "F7",
	[KeyF8] = "F8",
	[KeyF9] = "F9",
	[KeyF10] = "F10",
	[KeyF11] = "F11",
	[KeyF12] = "F12",
	[KeyPause] = "Pause",
	[KeyPrintScreen] = "Print Screen",
	[KeyNum0] = "Numeric '0'",
	[KeyNum1] = "Numeric '1'",
	[KeyNum2] = "Numeric '2'",
	[KeyNum3] = "Numeric '3'",
	[KeyNum4] = "Numeric '4'",
	[KeyNum5] = "Numeric '5'",
	[KeyNum6] = "Numeric '6'",
	[KeyNum7] = "Numeric '7'",
	[KeyNum8] = "Numeric '8'",
	[KeyNum9] = "Numeric '9'",
	[KeyNumDelete] = "Numeric Delete",
	[KeyNumEnter] = "Numeric Enter",
	[KeyNumMinus] = "Numeric '-'",
	[KeyNumSlash] = "Numeric '/'",
	[KeyLeftControl] = "Control (left)",
	[KeyRightControl] = "Control (right)",
	[KeyLeftShift] = "Shift (left)",
	[KeyRightShift] = "Shift (right)",
	[KeyLeftAlt] = "Alt (left)",
	[KeyRightAlt] = "Alt (right)",
	[KeyCapsLock] = "Caps Lock",
	[KeyNumLock] = "Num Lock",
	[KeyScrollLock] = "Scroll Lock",
};

static bool far _callback_keypress(GuiWindow_t window, GuiKeyEvent_t key_event)
{
	if(gui_get_keycode(key_event) == KeyEscape)
		exit(1);

#if 0
	if(gui_get_keycode(key_event) != 0)
	{
		//char buffer[256];
		const char far * name = key_names[gui_get_keycode(key_event)];
		if(name == NULL)
			name = "invalid";
		_fsnprintf(message_buffer, sizeof message_buffer, "[%d] Key: %Ws", keypress_counter++, name);
		gui_window_redraw(window);
	}
#endif
#if 0
	if(gui_get_keycode(key_event) != 0)
	{
		_fsnprintf(message_buffer, sizeof message_buffer, "[%d] Key: %d", keypress_counter++, gui_get_keycode(key_event));
		gui_window_redraw(window);
	}
#endif

	return true;
}

static bool far _callback_text(GuiWindow_t window, size_t count, char far * text)
{
#if __I86__
	_fsnprintf(message_buffer, sizeof message_buffer, "%.*Ws", (int)count, text);
#elif __386__
	_fsnprintf(message_buffer, sizeof message_buffer, "%.*s", (int)count, text);
#endif*/
	gui_window_redraw(window);
	return true;
}

static bool far _callback_buttonpress(GuiWindow_t window, GuiButtonEvent_t button_event)
{
	GuiPoint_t point = gui_get_button_coordinates(button_event);
	GuiMouseButton_t buttons = gui_get_buttons(button_event);
	_fsnprintf(message_buffer, sizeof message_buffer, "X (%d,%d): %X %s", point.x, point.y, buttons, gui_is_double_click(button_event) ? "D" : "S");
	gui_window_redraw(window);
	return true;
}

static bool far _callback_buttonrelease(GuiWindow_t window, GuiButtonEvent_t button_event)
{
	GuiPoint_t point = gui_get_button_coordinates(button_event);
	GuiMouseButton_t buttons = gui_get_buttons(button_event);
	_fsnprintf(message_buffer, sizeof message_buffer, "O (%d,%d): %X", point.x, point.y, buttons);
	gui_window_redraw(window);
	return true;
}

static bool far _callback_mousemove(GuiWindow_t window, GuiMouseEvent_t mouse_event)
{
	GuiPoint_t point = gui_get_mouse_coordinates(mouse_event);
	_fsnprintf(message_buffer, sizeof message_buffer, "M (%d,%d)", point.x, point.y);
	gui_window_redraw(window);
	return true;
}

int gui_main(GuiMainParameters_t parameters)
{
	int result;
	GuiWindow_t window;

	gui_init(&parameters);

	gui_message_box(GUINAME
#if __I86__
	" (16-bit)"
#elif __386__
# if __WINDOWS__ && !__NT__
	" (386)"
# else
	" (32-bit)"
# endif
#endif
	, "Greetings!", GUI_BUTTON(YES) | GUI_BUTTON(NO) | GUI_BUTTON(CANCEL) | GUI_BUTTON(HELP), GUI_BUTTON_NO, GUI_ICON_WARNING);

	gui_register_callback_show(_callback_show);
	gui_register_callback_keypress(_callback_keypress);
	gui_register_callback_text(_callback_text);
	gui_observe_buttons(GUI_BUTTON_LEFT | GUI_BUTTON_RIGHT | GUI_BUTTON_MIDDLE, GUI_SINGLE_CLICK | GUI_DOUBLE_CLICK);
//	gui_register_callback_buttonpress(_callback_buttonpress);
//	gui_register_callback_buttonrelease(_callback_buttonrelease);
//	gui_register_callback_mousemove(_callback_mousemove);

	window = gui_window_create(WINDOW_TITLE, 10, 20, 300, 150);

	result = gui_main_loop();

	gui_window_destroy(window);

	gui_terminate();

	return result;
}

