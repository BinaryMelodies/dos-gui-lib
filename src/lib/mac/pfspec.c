
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
			case mouseUp:
				// TODO
				break;
			case keyDown:
			case autoKey:
				if((event.modifiers & cmdKey) != 0)
				{
					if(event.what == keyDown)
					{
						doMenuCommand(MenuKey(event.message & charCodeMask));
					}
				}
				else
				{
					if(callback_key_press)
					{
						callback_key_press(FrontWindow(), event);
					}
				}
				break;
			case keyUp:
				if(callback_key_release)
				{
					callback_key_release(FrontWindow(), event);
				}
				break;
			case activateEvt:
				// TODO
				break;
			case updateEvt:
				if(callback_show)
				{
					callback_show((WindowPtr) event.message);
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
	GuiRectangle_t rectangle;
	rectangle.x = window->portRect.left;
	rectangle.y = window->portRect.top;
	rectangle.w = window->portRect.right - window->portRect.left;
	rectangle.h = window->portRect.bottom - window->portRect.top;
	return rectangle;
}

void gui_window_redraw(GuiWindow_t window)
{
#if !TARGET_API_MAC_CARBON
	WindowPtr savedPort;
	GetPort(&savedPort);
	SetPort(window);
	InvalRect(&window->portRect);
	SetPort(savedPort);
#else
	// TODO
#endif
}

void gui_window_begin_draw(GuiWindow_t window, GuiDrawContext_t * draw_context)
{
	BeginUpdate(window);
	GetPort(&draw_context->savedPort);
	SetPort(window);

/*
	EraseRect(&window->portRect);
	UpdateControls(window, window->visRgn);
	DrawGrowIcon(window);
*/

	draw_context->window = window;
	draw_context->font = 0;

	TextFont(draw_context->font);
	TextMode(srcCopy);
	TextSize(12);
}

void gui_window_end_draw(GuiDrawContext_t * draw_context)
{
	UpdateControls(draw_context->window, draw_context->window->visRgn);
	DrawGrowIcon(draw_context->window);

	SetPort(draw_context->savedPort);
	EndUpdate(draw_context->window);
}

#if TARGET_API_MAC_CARBON
extern pascal Pattern * GetQDGlobalsBlack(Pattern *);
#endif

void gui_set_color_black(GuiDrawContext_t * draw_context)
{
#if !TARGET_API_MAC_CARBON
	PenPat(&qd.black);
	BackPat(&qd.black);
#else
	Pattern black;
	PenPat(GetQDGlobalsBlack(&black));
	BackPat(&black);
#endif
}

#if TARGET_API_MAC_CARBON
extern pascal Pattern * GetQDGlobalsWhite(Pattern *);
#endif

void gui_set_color_white(GuiDrawContext_t * draw_context)
{
#if !TARGET_API_MAC_CARBON
	PenPat(&qd.white);
	BackPat(&qd.white);
#else
	Pattern white;
	PenPat(GetQDGlobalsWhite(&white));
	BackPat(&white);
#endif
}

void gui_fill_rectangle(GuiDrawContext_t * draw_context, int x, int y, int w, int h)
{
	Rect rect;
	rect.left = x;
	rect.top = y;
	rect.right = x + w;
	rect.bottom = y + h;
	PaintRect(&rect);
}

void gui_draw_line(GuiDrawContext_t * draw_context, int x1, int y1, int x2, int y2)
{
	MoveTo(x1, y1);
	LineTo(x2, y2);
}

int gui_get_font_height(GuiDrawContext_t * draw_context)
{
	FontInfo info;
	GetFontInfo(&info);
	return info.ascent + info.descent + info.leading;
}

void gui_write_text(GuiDrawContext_t * draw_context, int x, int y, const char * text)
{
	MoveTo(x, y);
	DrawText((char *)text, 0, strlen(text));
}

static const GuiKey_t virtual_codes[256] =
{
	'a', 's', 'd', 'f', 'h', 'g', 'z', 'x',
	'c', 'v', '`' /* ? */, 'b', 'q', 'w', 'e', 'r',
	'y', 't', '1', '2', '3', '4', '6', '5',
	'=', '9', '7', '-', '8', '0', ']', 'o',
	'u', '[', 'i', 'p', KeyEnter, 'l', 'j', '\\',
	'k', ';', '\\', ',', '/', 'n', 'm', '.',
	KeyTab, ' ', '`', KeyBackspace, 0, KeyEscape, KeyCapsLock /* ? */, KeyApple,
	KeyShift, KeyCapsLock, KeyAlt /* option */, KeyControl /* control */, KeyRight /* ApplKbd */, KeyDown /* ApplKbd */, KeyUp /* ApplKbd */,

	[0x41] = KeyNumDelete,
	[0x43] = KeyNumAsterisk,
	[0x45] = KeyNumPlus,
	[0x47] = KeyNumLock,
	[0x4B] = KeyNumSlash,
	[0x4C] = KeyNumEnter,
	[0x4E] = KeyNumMinus,
	//[0x51] = KeyNumEqual,
	[0x52] = KeyNum0,
	[0x53] = KeyNum1,
	[0x54] = KeyNum2,
	[0x55] = KeyNum3,
	[0x56] = KeyNum4,
	[0x57] = KeyNum5,
	[0x58] = KeyNum6,
	[0x59] = KeyNum7,
	[0x5B] = KeyNum8,
	[0x5C] = KeyNum9,

	[0x60] = KeyF5,
	[0x61] = KeyF6,
	[0x62] = KeyF7,
	[0x63] = KeyF3,
	[0x64] = KeyF8,
	[0x65] = KeyF9,
	[0x67] = KeyF11,
	[0x69] = KeyPrintScreen, // F13
	[0x6B] = KeyScrollLock, // F14
	[0x6D] = KeyF10,
	[0x6F] = KeyF12,
	[0x71] = KeyPause, // F15
	[0x72] = KeyInsert,
	[0x73] = KeyHome,
	[0x74] = KeyPageUp,
	[0x75] = KeyDelete,
	[0x76] = KeyF4,
	[0x77] = KeyEnd,
	[0x78] = KeyF2,
	[0x79] = KeyPageDown,
	[0x7A] = KeyF1,
	[0x7B] = KeyLeft,
	[0x7C] = KeyRight,
	[0x7D] = KeyDown,
	[0x7E] = KeyUp,
};

GuiKey_t gui_get_keycode(GuiKeyEvent_t event)
{
	return virtual_codes[(event.message & keyCodeMask) >> 8];
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

