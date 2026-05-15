
/* Implementation for IBM OS/2 */

#include "api.h"
#define _COMPILE_LIBRARY
#include "internal.h"

#include <stdlib.h>
#include <string.h>

// The window class name of all windows created by this library
#define WINDOW_CLASS_NAME "GenericMainWindow"

// Common variables needed for the library
HAB hab;
HMQ hmq;

// OS/2 vertical coordinates start from the bottom instead of the top, so for consistency sake, we will need to flip these values
static uint16_t fix_ycoord(HWND hwnd, uint16_t value)
{
	RECTL rcl;
	WinQueryWindowRect(hwnd, &rcl);
	return rcl.yTop - 1 - value;
}

// The window procedure for all windows created by this library
MRESULT EXPENTRY ClientWindowProcedure(
	HWND hwnd,
#if __I86__
	USHORT msg,
#else
	ULONG msg,
#endif
	MPARAM mparam1,
	MPARAM mparam2)
{
	switch(msg)
	{
	case WM_CHAR:
		if((CHARMSG(&msg)->fs & KC_KEYUP))
		{
			if((CHARMSG(&msg)->fs & KC_SCANCODE))
			{
				if(callback_key_release && callback_key_release(hwnd, *CHARMSG(&msg)))
					return 0;
			}
		}
		else
		{
			bool handled = false;
			if((CHARMSG(&msg)->fs & KC_SCANCODE) && callback_key_press && callback_key_press(hwnd, *CHARMSG(&msg)))
				handled = true;
			if(callback_text && (CHARMSG(&msg)->fs & KC_CHAR) != 0)
			{
				char c = CHARMSG(&msg)->chr;
				if(callback_text(hwnd, 1, &c))
					handled = true;
			}
			if(handled)
				return 0;
		}
		break;
	case WM_BUTTON1DOWN:
	case WM_BUTTON2DOWN:
	case WM_BUTTON3DOWN:
		if(callback_mouse_button_press)
		{
			GuiMouseButtonEvent_t event;
			event.msg = *MOUSEMSG(&msg);
			event.msg.y = fix_ycoord(hwnd, event.msg.y);
			event.button = msg == WM_BUTTON1DOWN ? GUI_MOUSE_BUTTON_LEFT : msg == WM_BUTTON2DOWN ? GUI_MOUSE_BUTTON_MIDDLE : GUI_MOUSE_BUTTON_RIGHT;
			event.double_click = false;
			if(callback_mouse_button_press(hwnd, event))
				return 0;
		}
		break;
	case WM_BUTTON1DBLCLK:
	case WM_BUTTON2DBLCLK:
	case WM_BUTTON3DBLCLK:
		if(callback_mouse_button_press)
		{
			GuiMouseButtonEvent_t event;
			event.msg = *MOUSEMSG(&msg);
			event.msg.y = fix_ycoord(hwnd, event.msg.y);
			event.button = msg == WM_BUTTON1DBLCLK ? GUI_MOUSE_BUTTON_LEFT : msg == WM_BUTTON2DBLCLK ? GUI_MOUSE_BUTTON_MIDDLE : GUI_MOUSE_BUTTON_RIGHT;
			event.double_click = true;
			if(callback_mouse_button_press(hwnd, event))
				return 0;
		}
		break;
	case WM_BUTTON1UP:
	case WM_BUTTON2UP:
	case WM_BUTTON3UP:
		if(callback_mouse_button_release)
		{
			GuiMouseButtonEvent_t event;
			event.msg = *MOUSEMSG(&msg);
			event.msg.y = fix_ycoord(hwnd, event.msg.y);
			event.button = msg == WM_BUTTON1UP ? GUI_MOUSE_BUTTON_LEFT : msg == WM_BUTTON2UP ? GUI_MOUSE_BUTTON_MIDDLE : GUI_MOUSE_BUTTON_RIGHT;
			event.double_click = false;
			if(callback_mouse_button_release(hwnd, event))
				return 0;
		}
		break;
	case WM_MOUSEMOVE:
		if(callback_mouse_move)
		{
			GuiMouseMoveEvent_t event = *MOUSEMSG(&msg);
			event.y = fix_ycoord(hwnd, event.y);
			if(callback_mouse_move(hwnd, event))
				return 0;
		}
		break;
	case WM_PAINT:
		if(callback_show && callback_show(hwnd))
			return 0;
		else
		{
			HPS hps;
			hps = WinBeginPaint(hwnd, (HPS)0, NULL);
			WinEndPaint(hps);
			return 0;
		}
		break;
// TODO: QUIT
	}

	return WinDefWindowProc(hwnd, msg, mparam1, mparam2);
}

void gui_init(GuiMainParameters_t * parameters)
{
	hab = WinInitialize(0);
	hmq = WinCreateMsgQueue(hab, 0);

	if(!WinRegisterClass(hab, WINDOW_CLASS_NAME, ClientWindowProcedure, CS_SIZEREDRAW, 0))
	{
		WinMessageBox(HWND_DESKTOP, HWND_DESKTOP, "WinRegisterClass failed", "Error", 0, 0);
		exit(1);
	}
}

void gui_terminate(void)
{
	WinDestroyMsgQueue(hmq);
	WinTerminate(hab);
}

int gui_message_box(const char * title, const char * message, GuiMessageBoxButtonSet_t buttons, int default_button, GuiMessageBoxIcon_t icon)
{
#if __I86__
	UINT type;
#elif __386__
	ULONG type;
#endif
	int result;

	if((buttons & GUI_MSGBOX_BUTTON(ABORT)) || (buttons & GUI_MSGBOX_BUTTON(IGNORE)))
	{
		type = MB_ABORTRETRYIGNORE;
		switch(default_button)
		{
		case GUI_MSGBOX_BUTTON_ABORT:
			type |= MB_DEFBUTTON1;
			break;
		case GUI_MSGBOX_BUTTON_RETRY:
			type |= MB_DEFBUTTON2;
			break;
		case GUI_MSGBOX_BUTTON_IGNORE:
			type |= MB_DEFBUTTON3;
			break;
		}
	}
	else if((buttons & GUI_MSGBOX_BUTTON(RETRY)))
	{
		type = MB_RETRYCANCEL;
		switch(default_button)
		{
		case GUI_MSGBOX_BUTTON_RETRY:
			type |= MB_DEFBUTTON1;
			break;
		case GUI_MSGBOX_BUTTON_CANCEL:
			type |= MB_DEFBUTTON2;
			break;
		}
	}
	else if((buttons & GUI_MSGBOX_BUTTON(YES)) || (buttons & GUI_MSGBOX_BUTTON(NO)))
	{
		if((buttons & GUI_MSGBOX_BUTTON(CANCEL)))
			type = MB_YESNOCANCEL;
		else
			type = MB_YESNO;

		switch(default_button)
		{
		case GUI_MSGBOX_BUTTON_YES:
			type |= MB_DEFBUTTON1;
			break;
		case GUI_MSGBOX_BUTTON_NO:
			type |= MB_DEFBUTTON2;
			break;
		case GUI_MSGBOX_BUTTON_CANCEL:
			if((buttons & GUI_MSGBOX_BUTTON(CANCEL)))
				type |= MB_DEFBUTTON3;
			break;
		}
	}
	else
	{
		if((buttons & GUI_MSGBOX_BUTTON(CANCEL)))
			type = MB_OKCANCEL;
		else
			type = MB_OK;

		switch(default_button)
		{
		case GUI_MSGBOX_BUTTON_OK:
			type |= MB_DEFBUTTON1;
			break;
		case GUI_MSGBOX_BUTTON_CANCEL:
			if((buttons & GUI_MSGBOX_BUTTON(CANCEL)))
				type |= MB_DEFBUTTON2;
			break;
		}
	}

	if((buttons & GUI_MSGBOX_BUTTON(HELP)))
		type |= MB_HELP;

	switch(icon)
	{
	case GUI_MSGBOX_ICON_NONE:
		break;
	case GUI_MSGBOX_ICON_WARNING:
		type |= MB_WARNING;
		break;
	case GUI_MSGBOX_ICON_QUESTION:
		type |= MB_QUERY;
		break;
	case GUI_MSGBOX_ICON_STOP:
		type |= MB_CRITICAL;
		break;
	case GUI_MSGBOX_ICON_INFORMATION:
		type |= MB_INFORMATION;
		break;
	}

	result = WinMessageBox(HWND_DESKTOP, (HWND)0, message, title, 0, type);

	switch(result)
	{
	case MBID_OK:
		result = GUI_MSGBOX_BUTTON_OK;
		break;
	case MBID_CANCEL:
		result = GUI_MSGBOX_BUTTON_CANCEL;
		break;
	case MBID_ABORT:
		result = GUI_MSGBOX_BUTTON_ABORT;
		break;
	case MBID_RETRY:
		result = GUI_MSGBOX_BUTTON_RETRY;
		break;
	case MBID_IGNORE:
		result = GUI_MSGBOX_BUTTON_IGNORE;
		break;
	case MBID_YES:
		result = GUI_MSGBOX_BUTTON_YES;
		break;
	case MBID_NO:
		result = GUI_MSGBOX_BUTTON_NO;
		break;
	default:
		result = -1;
		break;
	}

	return result;
}

GuiWindow_t gui_window_create(const char * window_title, int x, int y, int w, int h)
{
	HWND hwndFrame;
	HWND hwndClient;

	ULONG flags = FCF_TITLEBAR | FCF_SYSMENU | FCF_SIZEBORDER | FCF_MINMAX | FCF_SHELLPOSITION | FCF_TASKLIST;
	bool query_window_pos = false;

	hwndFrame = WinCreateStdWindow(HWND_DESKTOP, WS_VISIBLE, &flags, WINDOW_CLASS_NAME, window_title, 0L, (HMODULE)0, 0, &hwndClient);
	WinSendMsg(hwndFrame, WM_SETICON, (void *)WinQuerySysPointer(HWND_DESKTOP, SPTR_APPICON, FALSE), NULL);

	// TODO: ideally this should take place in the WM_CREATE callback

	flags = 0;

	if(y != GUI_WINPOS_DEFAULT)
	{
		// y needs to be flipped
		query_window_pos = true;
	}

	if(x != GUI_WINPOS_DEFAULT || y != GUI_WINPOS_DEFAULT)
	{
		flags |= SWP_MOVE;
		if(x == GUI_WINPOS_DEFAULT || y == GUI_WINPOS_DEFAULT)
		{
			query_window_pos = true;
		}
	}

	if(w != GUI_WINPOS_DEFAULT || h != GUI_WINPOS_DEFAULT)
	{
		flags |= SWP_SIZE;
		if(w == GUI_WINPOS_DEFAULT || h == GUI_WINPOS_DEFAULT)
		{
			query_window_pos = true;
		}
	}

	if(query_window_pos)
	{
		SWP swp;
		if(WinQueryWindowPos(hwndFrame, &swp))
		{
			if((flags & SWP_SIZE))
			{
				if(w == GUI_WINPOS_DEFAULT)
					w = swp.cx;
				if(h == GUI_WINPOS_DEFAULT)
					h = swp.cy;
			}

			if(y != GUI_WINPOS_DEFAULT)
			{
				// y must be flipped
				SWP swpParent;
				HWND hwndParent = (HWND)WinQueryWindow(hwndFrame, QW_PARENT
#if __I86__
					, FALSE
#endif
					);
				if(WinQueryWindowPos(hwndParent, &swpParent))
				{
					y = swpParent.cy - h - y;
				}
			}

			if((flags & SWP_MOVE))
			{
				if(x == GUI_WINPOS_DEFAULT)
					x = swp.x;
				if(y == GUI_WINPOS_DEFAULT)
					y = swp.y;
			}
		}
	}

	// TODO: handle GUI_WINPOS_DEFAULT (if left over) and GUI_WINPOS_MAXIMUM

	if(flags != 0)
	{
		WinSetWindowPos(hwndFrame, 0, x, y, w, h, flags);
	}

	return hwndClient;
}

int gui_main_loop(void)
{
	QMSG qmsg;
	while(WinGetMsg(hab, &qmsg, 0, 0, 0))
		WinDispatchMsg(hab, &qmsg);
	return 0;
}

void gui_terminate_main_loop(void)
{
	// TODO
}

void gui_window_destroy(GuiWindow_t win)
{
	WinDestroyWindow(
		(HWND)WinQueryWindow(win, QW_PARENT
#if __I86__
		, FALSE
#endif
		));
}

// OS/2 virtual codes are useless for determining which key has been pressed, so we need to use the scancodes
// This is also what SDL 1.2 does

static const GuiKey_t virtual_codes[256] =
{
	[1] = KeyEscape,
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', KeyBackspace,
	KeyTab, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', KeyEnter,
	KeyControl, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
	KeyShift, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', KeyRightShift,
	'*',
	KeyAlt,
	' ', KeyCapsLock,
	KeyF1, KeyF2, KeyF3, KeyF4, KeyF5, KeyF6, KeyF7, KeyF8, KeyF9, KeyF10,
	KeyNumLock, KeyScrollLock,
	KeyNum7, KeyNum8, KeyNum9, KeyNumMinus,
	KeyNum4, KeyNum5, KeyNum6, '+',
	KeyNum1, KeyNum2, KeyNum3, KeyNum0,
	KeyNumDelete,
	0, // (84)
	0, // (85)
	0, // (86)
	KeyF11, KeyF12,
	0, // (89)
	KeyNumEnter,
	KeyRightControl,
	KeyNumSlash,
	0, // (93)
	KeyRightAlt,
	0, // (95)
	KeyHome,
	KeyUp,
	KeyPageUp,
	KeyLeft,
	KeyRight,
	KeyEnd,
	KeyDown,
	KeyPageDown,
	KeyInsert,
	KeyDelete,
};

GuiKey_t gui_get_keycode(GuiKeyEvent_t event)
{
	GuiKey_t key;
	if(!(event.fs & KC_SCANCODE))
		return 0;
	else
		return virtual_codes[event.scancode];
}

GuiPoint_t gui_get_mouse_button_coordinates(GuiMouseButtonEvent_t event)
{
	GuiPoint_t point;
	point.x = event.msg.x;
	point.y = event.msg.y;
	return point;
}

GuiMouseButton_t gui_get_mouse_buttons(GuiMouseButtonEvent_t event)
{
	return event.button;
}

GuiMouseButton_t gui_is_double_click(GuiMouseButtonEvent_t event)
{
	return event.double_click;
}

GuiPoint_t gui_get_mouse_move_coordinates(GuiMouseMoveEvent_t event)
{
	GuiPoint_t point;
	point.x = event.x;
	point.y = event.y;
	return point;
}

GuiRectangle_t gui_window_get_client_area(GuiWindow_t window)
{
	GuiRectangle_t rectangle;
	RECTL rcl;

	WinQueryWindowRect(window, &rcl);
	rectangle.x = rcl.xLeft;
	rectangle.y = 0;
	rectangle.w = rcl.xRight - rcl.xLeft + 1;
	rectangle.h = rcl.yTop - rcl.yBottom + 1;

	return rectangle;
}

void gui_window_redraw(GuiWindow_t window)
{
	WinInvalidateRect(window, NULL, FALSE);
	WinUpdateWindow(window);
}

GuiDrawContext_t gui_window_begin_draw(GuiWindow_t window)
{
	GuiDrawContext_t draw_context;
	RECTL rcl;

	draw_context.hps = WinBeginPaint(window, (HPS)0, NULL);
//	GpiErase(draw_context.hps);

	WinQueryWindowRect(window, &rcl);
	draw_context.window_height = rcl.yTop;

	return draw_context;
}

void gui_window_end_draw(GuiDrawContext_t * draw_context)
{
	WinEndPaint(draw_context->hps);
}

void gui_set_color_black(GuiDrawContext_t * draw_context)
{
	GpiSetColor(draw_context->hps, CLR_BLACK);
}

void gui_set_color_white(GuiDrawContext_t * draw_context)
{
	GpiSetColor(draw_context->hps, CLR_WHITE);
}

void gui_fill_rectangle(GuiDrawContext_t * draw_context, int x, int y, int w, int h)
{
	POINTL ptl;

	ptl.x = x;
	ptl.y = draw_context->window_height - 1 - y;

	GpiMove(draw_context->hps, &ptl);

	ptl.x = x + w;
	ptl.y = draw_context->window_height - 1 - y - h;

	GpiBox(draw_context->hps, DRO_FILL, &ptl, 0L, 0L);
}

void gui_draw_line(GuiDrawContext_t * draw_context, int x1, int y1, int x2, int y2)
{
	POINTL ptl;

	ptl.x = x1;
	ptl.y = draw_context->window_height - 1 - y1;

	GpiMove(draw_context->hps, &ptl);

	ptl.x = x2;
	ptl.y = draw_context->window_height - 1 - y2;

	GpiLine(draw_context->hps, &ptl);
}

int gui_get_font_height(GuiDrawContext_t * draw_context)
{
	FONTMETRICS fm;
	GpiQueryFontMetrics(draw_context->hps, sizeof fm, &fm);
	return fm.lMaxBaselineExt;
}

void gui_write_text(GuiDrawContext_t * draw_context, int x, int y, const char * text)
{
	FONTMETRICS fm;
	POINTL ptl;

	GpiQueryFontMetrics(draw_context->hps, sizeof fm, &fm);

	ptl.x = x;
	ptl.y = draw_context->window_height - 1 - y + fm.lMaxDescender;
	GpiCharStringAt(draw_context->hps, &ptl, strlen(text), (char *)text);
	//GpiMove(draw_context, &ptl);
	//GpiCharString(draw_context->hps, strlen(text), (char *)text);
}

int main(int argc, char ** argv, char ** envp)
{
	GuiMainParameters_t parameters;
	parameters.argc = argc;
	parameters.argv = argv;
	parameters.envp = envp;
	return gui_main(parameters);
}

