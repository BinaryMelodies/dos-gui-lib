
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

static bool far _callback_show(GuiWindow_t window)
{
	GuiDrawContext_t draw_context = gui_window_begin_draw(window);
	GuiRectangle_t client_area = gui_window_get_client_area(window);

	gui_set_color_white(&draw_context);
	gui_fill_rectangle(&draw_context, 0, 0, client_area.w, client_area.h);
	gui_set_color_black(&draw_context);
	gui_draw_line(&draw_context, 10, 10, 100, 100);

	gui_window_end_draw(&draw_context);

	return true;
}

int gui_main(GuiMainParameters_t parameters)
{
	int result;
	GuiWindow_t window;
	GuiWidget_t button1, button2;

	gui_init(&parameters);

	gui_register_callback_show(_callback_show);

	window = gui_window_create(GUINAME
#if __I86__
	" (16-bit)",
#elif __386__
# if __WINDOWS__ && !__NT__
	" (386)",
# else
	" (32-bit)",
# endif
#endif
		10, 20, 400, 300);

	button1 = gui_create_push_button(window, GuiWindowRoot, 10, 10, 380, 20, "Button 1", 0);
	button2 = gui_create_push_button(window, GuiWindowRoot, 10, 35, 380, 20, "Button 2", 0);

	result = gui_main_loop();

	gui_window_destroy(window);

	gui_terminate();

	return result;
}

