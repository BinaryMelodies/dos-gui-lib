
/* Implementation for Commodore Amiga */
#include "api.h"
#define _COMPILE_LIBRARY
#include "internal.h"

#if __amigaos4__
# define LIBIF(__name) (__name)->
#else
# define LIBIF(__name)
#endif

static bool gui_running;
static struct Window * main_window; // TODO

void gui_init(GuiMainParameters_t * parameters)
{
}

void gui_terminate(void)
{
}

int gui_main_loop(void)
{
	gui_running = true;
	while(gui_running)
	{
		struct IntuiMessage * message;
		LIBIF(IExec) WaitPort(main_window->UserPort);
		while(message = (struct IntuiMessage *) LIBIF(IExec) GetMsg(main_window->UserPort))
		{
			switch(message->Class)
			{
			case IDCMP_CLOSEWINDOW:
				if(callback_close)
				{
					callback_close(main_window);
				}
				break;
			}
			LIBIF(IExec) ReplyMsg((struct Message *) message);
		}
	}
}

void gui_terminate_main_loop(void)
{
	gui_running = false;
}

int gui_message_box(const char * title, const char * message, GuiMessageBoxButtonSet_t buttons, int default_button, GuiMessageBoxIcon_t icon)
{
	// TODO
}

GuiWindow_t gui_window_create(const char * window_title, int x, int y, int w, int h, GuiWindowState_t state)
{
	ULONG tags[23];
	size_t tag_index = 0;
	struct Window * window;

	// TODO: check kickstart version and use OpenWindow instead

	if(x != GUI_WINPOS_DEFAULT) // TODO: max
	{
		tags[tag_index++] = WA_Left;
		tags[tag_index++] = x;
	}

	if(y != GUI_WINPOS_DEFAULT) // TODO: max
	{
		tags[tag_index++] = WA_Top;
		tags[tag_index++] = y;
	}

	if(w != GUI_WINPOS_DEFAULT) // TODO: max
	{
		tags[tag_index++] = WA_Width;
		tags[tag_index++] = w;
	}

	if(h != GUI_WINPOS_DEFAULT) // TODO: max
	{
		tags[tag_index++] = WA_Height;
		tags[tag_index++] = h;
	}

	if(window_title != NULL)
	{
		tags[tag_index++] = WA_Title;
		tags[tag_index++] = (ULONG) window_title;
	}

	tags[tag_index++] = WA_Activate;
	tags[tag_index++] = true;
	tags[tag_index++] = WA_IDCMP;
	tags[tag_index++] = IDCMP_CLOSEWINDOW;
	tags[tag_index++] = WA_CloseGadget;
	tags[tag_index++] = true;
	tags[tag_index++] = WA_SizeGadget;
	tags[tag_index++] = true;
	tags[tag_index++] = WA_DepthGadget;
	tags[tag_index++] = true;
	tags[tag_index++] = WA_DragBar;
	tags[tag_index++] = true;

	tags[tag_index] = TAG_END;

	window = LIBIF(IIntuition) OpenWindowTagList(NULL, (struct TagItem *) &tags);
	// TODO: do not single out the first window
	if(main_window == NULL)
	{
		main_window = window;
	}

	return window;
}

void gui_window_show(GuiWindow_t window, GuiWindowState_t state, GuiWindowStateAction_t action)
{
	// TODO
}

void gui_window_destroy(GuiWindow_t window)
{
	LIBIF(IIntuition) CloseWindow(window);
}

GuiRectangle_t gui_window_get_client_area(GuiWindow_t window)
{
	// TODO
}

void gui_window_redraw(GuiWindow_t window)
{
	// TODO
}

GuiDrawContext_t gui_window_begin_draw(GuiWindow_t window)
{
	// TODO
}

void gui_window_end_draw(GuiDrawContext_t * draw_context)
{
	// TODO
}

void gui_set_color_black(GuiDrawContext_t * draw_context)
{
	// TODO
}

void gui_set_color_white(GuiDrawContext_t * draw_context)
{
	// TODO
}

void gui_fill_rectangle(GuiDrawContext_t * draw_context, int x, int y, int w, int h)
{
	// TODO
}

void gui_draw_line(GuiDrawContext_t * draw_context, int x1, int y1, int x2, int y2)
{
	// TODO
}

int gui_get_font_height(GuiDrawContext_t * draw_context)
{
	// TODO
}

void gui_write_text(GuiDrawContext_t * draw_context, int x, int y, const char * text)
{
	// TODO
}

GuiKey_t gui_get_keycode(GuiKeyEvent_t event)
{
	// TODO
}

GuiPoint_t gui_get_mouse_button_coordinates(GuiMouseButtonEvent_t event)
{
	// TODO
}

GuiMouseButton_t gui_get_mouse_buttons(GuiMouseButtonEvent_t event)
{
	// TODO
}

GuiMouseButton_t gui_is_double_click(GuiMouseButtonEvent_t event)
{
	// TODO
}

GuiPoint_t gui_get_mouse_move_coordinates(GuiMouseMoveEvent_t event)
{
	// TODO
}

GuiWidget_t gui_create_push_button(GuiWindow_t window, GuiWidget_t parent, int x, int y, int w, int h, const char far * caption, long flags)
{
	// TODO
}

int main(int argc, char * argv[])
{
	GuiMainParameters_t parameters;
	parameters.argc = argc;
	parameters.argv = argv;
	return gui_main(parameters);
}

