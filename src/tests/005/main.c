
#include "api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libc.h"

#if GUI_WIN || GUI_MGW
# define GUINAME "Windows"
#elif GUI_GEM
# define GUINAME "GEM"
#elif GUI_TOS
# define GUINAME "Atari TOS"
#elif GUI_DVX
# define GUINAME "DESQview/X"
#elif GUI_OS2
# define GUINAME "OS/2"
#elif GUI_LNX
# define GUINAME "POSIX/X11"
#endif

#define WINDOW_TITLE "Test in " GUINAME

char message_buffer[256] = "Brave new world!";

static size_t window_count = 2;
static bool window1_alive = true;
static GuiWindow_t window1;
static bool window2_alive = true;
static GuiWindow_t window2;

static bool far _callback_close(GuiWindow_t window)
{
	if(window == window1)
	{
		window1_alive = false;
	}
	else if(window == window2)
	{
		window2_alive = false;
	}

	gui_window_destroy(window);
	if(--window_count == 0)
	{
		gui_terminate_main_loop();
	}
	return true;
}

static bool far _callback_show(GuiWindow_t window)
{
	GuiDrawContext_t draw_context;
	GuiRectangle_t client_area;
	gui_window_begin_draw(window, &draw_context);
	client_area = gui_window_get_client_area(window);

	gui_set_color_black(&draw_context);
	gui_fill_rectangle(&draw_context, 0, 0, client_area.w, client_area.h);
	/*gui_set_color_white(&draw_context);
	gui_draw_line(&draw_context, 10, 10, 30, 30);
	gui_write_text(&draw_context, 10, 20, message_buffer);*/
	gui_window_end_draw(&draw_context);

	return true;
}

static bool far _callback_keypress(GuiWindow_t window, GuiKeyEvent_t key_event)
{
	if(gui_get_keycode(key_event) == KeyEscape)
		_callback_close(window);
	else if(gui_get_keycode(key_event) == '1')
		gui_window_activate(window1);
	else if(gui_get_keycode(key_event) == '2')
		gui_window_activate(window2);

	return true;
}

int gui_main(GuiMainParameters_t parameters)
{
	int result;

	gui_init(&parameters);

	gui_register_callback_close(_callback_close);
	gui_register_callback_show(_callback_show);
	gui_register_callback_key_press(_callback_keypress);

	window1 = gui_window_create(NULL, 10, 20, 300, 150, GUI_WINDOW_STATE_DEFAULT);
	window2 = gui_window_create(NULL, 20, 30, 300, 150, GUI_WINDOW_STATE_DEFAULT);

	result = gui_main_loop();

	if(window_count != 0)
	{
		if(window1_alive)
			gui_window_destroy(window1);
		if(window2_alive)
			gui_window_destroy(window2);
	}

	gui_terminate();

	return result;
}

