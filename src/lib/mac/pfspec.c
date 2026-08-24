
/* Implementation for Apple Macintosh */

#include "api.h"
#define _COMPILE_LIBRARY
#include "internal.h"

#include <stdlib.h>
#include <string.h>

enum
{
	zoomDocProc = 8,
};

enum
{
	kMenuApple = 128,
	kMenuFile,
};

enum
{
	kItemQuit = 1,
};

static bool color_qd_available;
static volatile bool gui_running;

static void doMenuCommand(long menuResult)
{
	Str255 itemName;
	short menuID = menuResult >> 16;
	short menuItem = menuResult & 0xFFFF;
	switch(menuID)
	{
	case kMenuApple:
#if !TARGET_API_MAC_CARBON
		GetMenuItemText(GetMenuHandle(kMenuApple), menuItem, itemName);
		OpenDeskAcc(itemName);
#endif
		break;
	case kMenuFile:
		switch(menuItem)
		{
		case kItemQuit:
			gui_terminate_main_loop();
			break;
		}
		break;
	}
	HiliteMenu(0);
}

void gui_init(GuiMainParameters_t * parameters)
{
	LONGINT response;

#if !TARGET_API_MAC_CARBON
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(NULL);
#endif
	InitCursor();

	if(Gestalt(gestaltQuickdrawVersion, &response) == noErr)
	{
		// TODO: Inside Macintosh seems to recommend checking gestalt32BitQD13 instead and not relying on the gestaltHasColor bit, calling it unreliable
		if((response & 0xFFFF) >= gestalt8BitQD)
		{
			color_qd_available =
				Gestalt(gestaltQuickdrawFeatures, &response) == noErr
				&& (response & (1 << gestaltHasColor)) != 0;
		}
	}

	// create default menus programmatically
	MenuHandle menuAppleHandle = NewMenu(kMenuApple, "\p\x14");
	InsertMenu(menuAppleHandle, 0);
	AppendMenu(menuAppleHandle, "\p-");

	MenuHandle menuFileHandle = NewMenu(kMenuFile, "\pFile");
	InsertMenu(menuFileHandle, 0);
	AppendMenu(menuFileHandle, "\pQuit/Q");
}

void gui_terminate(void)
{
	ExitToShell();
}

int gui_main_loop(void)
{
	gui_running = true;
	DrawMenuBar();
	while(gui_running)
	{
		WindowRef thisWindow;
		EventRecord event;
		int part;

		if(WaitNextEvent(everyEvent, &event, 180, NULL)) // TODO: decide if these parameters are the best
		{
			switch(event.what)
			{
			case mouseDown:
				switch(part = FindWindow(event.where, &thisWindow))
				{
				case inMenuBar:
					doMenuCommand(MenuSelect(event.where));
					break;
#if !TARGET_API_MAC_CARBON
				case inSysWindow:
					SystemClick(&event, thisWindow);
					break;
#endif
				case inDrag:
#if !TARGET_API_MAC_CARBON
					DragWindow(thisWindow, event.where, &qd.screenBits.bounds);
#else
					{
						BitMap screenBits;
						GetQDGlobalsScreenBits(&screenBits);
						DragWindow(thisWindow, event.where, &screenBits.bounds);
					}
#endif
				case inGrow:
					// TODO
					break;
				case inGoAway:
					if(TrackGoAway(thisWindow, event.where))
					{
						if(callback_close)
						{
							callback_close(thisWindow);
						}
					}
					break;
				case inZoomIn:
				case inZoomOut:
					if(TrackBox(thisWindow, event.where, part))
					{
						// TODO
					}
					break;
				}
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
	Rect windowRect;
	WindowRef window;

	windowRect.left = x;
	windowRect.top = GetMBarHeight() + y;
	windowRect.right = x + w;
	windowRect.bottom = y + h;

	unsigned char * pascal_title = malloc(strlen(window_title) + 1);
	pascal_title[0] = strlen(window_title);
	memcpy(&pascal_title[1], window_title, strlen(window_title));

	if(color_qd_available)
	{
		window = NewCWindow(
			NULL,
			&windowRect,
			pascal_title,
			true,
			zoomDocProc,
			(WindowPtr) -1,
			true,
			0);
	}
	else
	{
		window = NewWindow(
			NULL,
			&windowRect,
			pascal_title,
			true,
			zoomDocProc,
			(WindowPtr) -1,
			true,
			0);
	}
	free(pascal_title);

	return window;
}

void gui_window_show(GuiWindow_t window, GuiWindowState_t state, GuiWindowStateAction_t action)
{
	// TODO
}

void gui_window_destroy(GuiWindow_t window)
{
	DisposeWindow(window);
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

int main(void)
{
	GuiMainParameters_t parameters;
	return gui_main(parameters);
}

