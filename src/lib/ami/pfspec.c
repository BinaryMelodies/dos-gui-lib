
/* Implementation for Commodore Amiga */
#include "api.h"
#define _COMPILE_LIBRARY
#include "internal.h"
#include <string.h>

#if __amigaos4__
# define LIBIF(__name) (__name)->
#else
# define LIBIF(__name)
#endif

static volatile bool gui_running;
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
		//LIBIF(IExec) Wait(1 << main_window->UserPort->mp_SigBit); // TODO: check?
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
			//case IDCMP_NEWSIZE:
			//case IDCMP_CHANGEWINDOW:
			//case IDCMP_ACTIVEWINDOW:
			case IDCMP_REFRESHWINDOW:
				if(callback_show)
				{
					LIBIF(IIntuition) BeginRefresh(main_window); // TODO: it should be up to the callback to invoke this
					callback_show(main_window);
					LIBIF(IIntuition) EndRefresh(main_window, true); // TODO: it should be up to the callback to invoke this
				}
				break;
			case IDCMP_RAWKEY:
				if((message->Code & 0x80) == 0)
				{
					if(callback_key_press)
					{
						callback_key_press(main_window, *message);
					}
				}
				else
				{
					if(callback_key_release)
					{
						callback_key_release(main_window, *message);
					}
				}
				break;
			}

			LIBIF(IExec) ReplyMsg((struct Message *) message);

			if(!gui_running)
			{
				break;
			}
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
	ULONG events = 0;
	size_t tag_index = 0;
	struct Window * window;

	// TODO: check kickstart version and use OpenWindow instead

	if(callback_close)
	{
		events |= IDCMP_CLOSEWINDOW;
	}
	if(callback_show)
	{
		events |= IDCMP_REFRESHWINDOW;
	}
	if(callback_key_press || callback_key_release)
	{
		events |= IDCMP_RAWKEY;
	}

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
	tags[tag_index++] = events;
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

	if(callback_show)
	{
		callback_show(window);
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
	GuiRectangle_t rectangle;
	rectangle.x = 0;
	rectangle.y = 0;
	rectangle.w = window->Width;
	rectangle.h = window->Height;
	return rectangle;
}

void gui_window_redraw(GuiWindow_t window)
{
	if(callback_show)
	{
		callback_show(main_window);
	}
}

void gui_window_begin_draw(GuiWindow_t window, GuiDrawContext_t * draw_context)
{
//	LIBIF(IIntuition) BeginRefresh(window); // TODO: must not be called on the first invocation, outside refreshing
	draw_context->window = window;
	draw_context->rastPort = window->RPort;
	LIBIF(IGraphics) SetDrMd(draw_context->rastPort, JAM1);
}

void gui_window_end_draw(GuiDrawContext_t * draw_context)
{
//	LIBIF(IIntuition) EndRefresh(draw_context->window, true); // TODO: must not be called on the first invocation, outside refreshing
	LIBIF(IIntuition) RefreshWindowFrame(draw_context->window);
}

void gui_set_color_black(GuiDrawContext_t * draw_context)
{
#if __m68k__
	LIBIF(IGraphics) SetAPen(draw_context->rastPort, 1);
	LIBIF(IGraphics) SetBPen(draw_context->rastPort, 1);
#else
	LIBIF(IGraphics) SetRPAttrs(draw_context->rastPort,
		RPTAG_APenColor, 0xFF000000,
		RPTAG_BPenColor, 0xFF000000,
		RPTAG_DrMd, JAM1,
		TAG_END);
#endif
}

void gui_set_color_white(GuiDrawContext_t * draw_context)
{
#if __m68k__
	LIBIF(IGraphics) SetAPen(draw_context->rastPort, 2);
	LIBIF(IGraphics) SetBPen(draw_context->rastPort, 2);
#else
	LIBIF(IGraphics) SetRPAttrs(draw_context->rastPort,
		RPTAG_APenColor, 0xFFFFFFFF,
		RPTAG_BPenColor, 0xFFFFFFFF,
		RPTAG_DrMd, JAM1,
		TAG_END);
#endif
}

void gui_fill_rectangle(GuiDrawContext_t * draw_context, int x, int y, int w, int h)
{
	LIBIF(IGraphics) RectFill(draw_context->rastPort, x, y, x + w, y + h);
}

void gui_draw_line(GuiDrawContext_t * draw_context, int x1, int y1, int x2, int y2)
{
	LIBIF(IGraphics) Move(draw_context->rastPort, x1, y1);
	LIBIF(IGraphics) Draw(draw_context->rastPort, x2, y2);
}

int gui_get_font_height(GuiDrawContext_t * draw_context)
{
	// TODO
}

void gui_write_text(GuiDrawContext_t * draw_context, int x, int y, const char * text)
{
	LIBIF(IGraphics) Move(draw_context->rastPort, x, y);
	LIBIF(IGraphics) Text(draw_context->rastPort, text, strlen(text));
}

static const GuiKey_t virtual_codes[256] =
{
	'`', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', '0', '-', '=', '\\', 0, KeyNum0,
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
	'o', 'p', '[', ']', 0, KeyNum1, KeyNum2, KeyNum3,
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k',
	'l', ';', '\'', 0, 0 /* unnamed key, UAE maps insert */, KeyNum4, KeyNum5, KeyNum6,
	0 /* unnamed key */, 'z', 'x', 'c', 'v', 'b', 'n', 'm',
	',', '.', '/', 0, KeyNumDelete, KeyNum7, KeyNum8, KeyNum9,
	' ', KeyBackspace, KeyTab, KeyNumEnter, KeyEnter, KeyEscape, KeyDelete, 0,
	0, 0, KeyNumMinus, 0, KeyUp, KeyDown, KeyRight, KeyLeft,
	KeyF1, KeyF2, KeyF3, KeyF4, KeyF5, KeyF6, KeyF7, KeyF8,
	KeyF9, KeyF10, 0 /* Num (, UAE maps home */, 0 /* Num ), UAE maps pgup */, KeyNumSlash, KeyNumAsterisk, KeyNumPlus, 0 /* Help, UAE maps end */,
	KeyShift, KeyRightShift, KeyCapsLock, KeyControl, KeyAlt, 0, KeyVendor, KeyRightVendor /* UAE maps pgdn */
};

GuiKey_t gui_get_keycode(GuiKeyEvent_t event)
{
	return virtual_codes[event.Code & 0x7F];
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

