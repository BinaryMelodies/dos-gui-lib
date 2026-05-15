
/* Implementation for Microsoft Windows */

#include "api.h"
#define _COMPILE_LIBRARY
#include "internal.h"

#include <stdlib.h>
#include <string.h>
#include <i86.h>

// The window class name of all windows created by this library
#define WINDOW_CLASS_NAME "GenericMainWindow"

// SetTextAlign was introduced in Windows 2.0, so including it at compile time would break the code on Windows 1.x
// Instead, we load the procedure once we have checked that the Windows version is recent enough
#if __I86__ && WINDOWS_TARGET_VERSION < 2
UINT (FAR PASCAL * _SetTextAlign)(HDC, UINT) = NULL;
#endif

// Common variables needed for the library
HINSTANCE hInstance;
HINSTANCE hPrevInstance;
LPSTR lpCmdLine;
int nCmdShow;

// The window procedure for all windows created by this library
LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_KEYDOWN:
		{
			GuiKeyEvent_t event;
			event.wParam = wParam;
			event.lParam = lParam;
			if(callback_key_press && callback_key_press(hWnd, event))
				return 0;
		}
		break;
	case WM_CHAR:
		if(callback_text)
		{
			char c = wParam;
			if(callback_text(hWnd, 1, &c))
				return 0;
		}
		break;
	case WM_KEYUP:
		{
			GuiKeyEvent_t event;
			event.wParam = wParam;
			event.lParam = lParam;
			if(callback_key_release && callback_key_release(hWnd, event))
				return 0;
		}
		break;
	case WM_LBUTTONDOWN: // TODO: only the selected buttons
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		if(callback_mouse_button_press)
		{
			GuiMouseButtonEvent_t event;
			event.wParam = wParam;
			event.lParam = lParam;
			event.double_click = false;
			if(callback_mouse_button_press(hWnd, event))
				return 0;
		}
		break;
	case WM_LBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
		if(callback_mouse_button_press)
		{
			GuiMouseButtonEvent_t event;
			event.wParam = wParam;
			event.lParam = lParam;
			event.double_click = true;
			if(callback_mouse_button_press(hWnd, event))
				return 0;
		}
		break;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		if(callback_mouse_button_release)
		{
			GuiMouseButtonEvent_t event;
			event.wParam = wParam;
			event.lParam = lParam;
			event.double_click = false;
			if(callback_mouse_button_release(hWnd, event))
				return 0;
		}
		break;
	case WM_MOUSEMOVE:
		if(callback_mouse_move)
		{
			GuiMouseMoveEvent_t event;
			event.wParam = wParam;
			event.lParam = lParam;
			if(callback_mouse_move(hWnd, event))
				return 0;
		}
		break;
	case WM_PAINT:
		if(callback_show && callback_show(hWnd))
			return 0;
		break;
	case WM_COMMAND:
		if(callback_action)
			callback_action(hWnd, LOWORD(wParam), GUI_ACTION_CLICKED);
		return 0;

	//case WM_CLOSE: // TODO: should precede WM_DESTROY?
	case WM_DESTROY:
		if(callback_quit && callback_quit(hWnd))
			return 0;
		gui_terminate_main_loop();
		return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

void gui_init(GuiMainParameters_t * parameters)
{
	hInstance = parameters->hInstance;
	hPrevInstance = parameters->hPrevInstance;
	lpCmdLine = parameters->lpCmdLine;
	nCmdShow = parameters->nCmdShow;

#if __I86__ && WINDOWS_TARGET_VERSION < 2
	if(LOBYTE(LOWORD(GetVersion())) >= 2) // check major version
	{
		// not supported on Windows 1.x
		_SetTextAlign = (LPVOID)GetProcAddress(GetModuleHandle("GDI"), (LPSTR)346 /*SetTextAlign*/);
	}
#endif

#if __I86__
	if(!hPrevInstance)
#endif
	{
		WNDCLASS wc;
		memset(&wc, 0, sizeof wc);

		wc.lpfnWndProc = (LPVOID) MainWindowProc;
		wc.hInstance = hInstance;
		wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
		wc.lpszClassName = WINDOW_CLASS_NAME;
		wc.style = CS_DBLCLKS; // captures double clicks (TODO: does not work)

		if(!RegisterClass(&wc))
		{
			MessageBox(NULL, "RegisterClass failed", "Error", MB_ICONHAND | MB_OK);
			exit(1);
		}
	}
}

void gui_terminate(void)
{
}

int gui_message_box(const char * title, const char * message, GuiMessageBoxButtonSet_t buttons, int default_button, GuiMessageBoxIcon_t icon)
{
	UINT type;
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
#if __NT__
		case GUI_MSGBOX_BUTTON_HELP:
			if((buttons & GUI_MSGBOX_BUTTON(HELP)))
				type |= MB_DEFBUTTON4;
#endif
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
#if __NT__
		case GUI_MSGBOX_BUTTON_HELP:
			if((buttons & GUI_MSGBOX_BUTTON(HELP)))
				type |= MB_DEFBUTTON3;
			break;
#endif
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
#if __NT__
		case GUI_MSGBOX_BUTTON_HELP:
			if((buttons & GUI_MSGBOX_BUTTON(HELP)))
				type |= buttons & GUI_MSGBOX_BUTTON(CANCEL) ? MB_DEFBUTTON4 : MB_DEFBUTTON3;
			break;
#endif
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
#if __NT__
		case GUI_MSGBOX_BUTTON_HELP:
			if((buttons & GUI_MSGBOX_BUTTON(HELP)))
				type |= buttons & GUI_MSGBOX_BUTTON(CANCEL) ? MB_DEFBUTTON3 : MB_DEFBUTTON2;
			break;
#endif
		}
	}

#if __NT__
	if((buttons & GUI_MSGBOX_BUTTON(HELP)))
		type |= MB_HELP;
#endif

	switch(icon)
	{
	case GUI_MSGBOX_ICON_NONE:
		break;
	case GUI_MSGBOX_ICON_WARNING:
		type |= MB_ICONEXCLAMATION;
		break;
	case GUI_MSGBOX_ICON_QUESTION:
		type |= MB_ICONQUESTION;
		break;
	case GUI_MSGBOX_ICON_STOP:
		type |= MB_ICONSTOP;
		break;
	case GUI_MSGBOX_ICON_INFORMATION:
		type |= MB_ICONINFORMATION;
		break;
	}

	result = MessageBox(NULL, message, title, type);

	switch(result)
	{
	case IDOK:
		result = GUI_MSGBOX_BUTTON_OK;
		break;
	case IDCANCEL:
		result = GUI_MSGBOX_BUTTON_CANCEL;
		break;
	case IDABORT:
		result = GUI_MSGBOX_BUTTON_ABORT;
		break;
	case IDRETRY:
		result = GUI_MSGBOX_BUTTON_RETRY;
		break;
	case IDIGNORE:
		result = GUI_MSGBOX_BUTTON_IGNORE;
		break;
	case IDYES:
		result = GUI_MSGBOX_BUTTON_YES;
		break;
	case IDNO:
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
	HWND hWnd;

	if(x == GUI_WINPOS_DEFAULT)
		x = CW_USEDEFAULT;
	else if(x == GUI_WINPOS_MAXIMUM)
		x = 0; // TODO

	if(y == GUI_WINPOS_DEFAULT)
		y = CW_USEDEFAULT;
	else if(y == GUI_WINPOS_MAXIMUM)
		y = 0; // TODO

	if(w == GUI_WINPOS_DEFAULT)
		w = CW_USEDEFAULT;
	else if(w == GUI_WINPOS_MAXIMUM)
		w = 320; // TODO: set to screen width

	if(h == GUI_WINPOS_DEFAULT)
		h = CW_USEDEFAULT;
	else if(h == GUI_WINPOS_MAXIMUM)
		h = 200; // TODO: set to screen height

	hWnd = CreateWindow(WINDOW_CLASS_NAME, window_title, WS_OVERLAPPEDWINDOW,
		x, y, w, h, NULL, NULL, hInstance, NULL);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return hWnd;
}

int gui_main_loop(void)
{
	MSG msg;

	while(GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

void gui_terminate_main_loop(void)
{
	PostQuitMessage(0);
}

void gui_window_destroy(GuiWindow_t window)
{
	DestroyWindow(window);
}

static const GuiKey_t virtual_codes[256] =
{
	[VK_BACK] = KeyBackspace,
	[VK_TAB] = KeyTab,
	[VK_RETURN] = KeyEnter,
	[VK_SHIFT] = KeyShift,
	[VK_CONTROL] = KeyControl,
	[VK_PAUSE] = KeyPause,
	[VK_CAPITAL] = KeyCapsLock,
	[VK_ESCAPE] = KeyEscape,
	[VK_SPACE] = ' ',
	[VK_END] = KeyEnd,
	[VK_HOME] = KeyHome,
	[VK_LEFT] = KeyLeft,
	[VK_UP] = KeyUp,
	[VK_RIGHT] = KeyRight,
	[VK_DOWN] = KeyDown,
	[VK_PRINT] = KeyPrintScreen,
	[VK_INSERT] = KeyInsert,
	[VK_DELETE] = KeyDelete,

	['0'] = '0',
	['1'] = '1',
	['2'] = '2',
	['3'] = '3',
	['4'] = '4',
	['5'] = '5',
	['6'] = '6',
	['7'] = '7',
	['8'] = '8',
	['9'] = '9',
	[';'] = ';',
	['='] = '=',
	['A'] = 'a',
	['B'] = 'b',
	['C'] = 'c',
	['D'] = 'd',
	['E'] = 'e',
	['F'] = 'f',
	['G'] = 'g',
	['H'] = 'h',
	['I'] = 'i',
	['J'] = 'j',
	['K'] = 'k',
	['L'] = 'l',
	['M'] = 'm',
	['N'] = 'n',
	['O'] = 'o',
	['P'] = 'p',
	['Q'] = 'q',
	['R'] = 'r',
	['S'] = 's',
	['T'] = 't',
	['U'] = 'u',
	['V'] = 'v',
	['W'] = 'w',
	['X'] = 'x',
	['Y'] = 'y',
	['Z'] = 'z',

	[VK_NUMPAD0] = KeyNum0,
	[VK_NUMPAD1] = KeyNum1,
	[VK_NUMPAD2] = KeyNum2,
	[VK_NUMPAD3] = KeyNum3,
	[VK_NUMPAD4] = KeyNum4,
	[VK_NUMPAD5] = KeyNum5,
	[VK_NUMPAD6] = KeyNum6,
	[VK_NUMPAD7] = KeyNum7,
	[VK_NUMPAD8] = KeyNum8,
	[VK_NUMPAD9] = KeyNum9,
	[VK_MULTIPLY] = '*',
	[VK_ADD] = '+',
	[VK_SUBTRACT] = KeyNumMinus,
	[VK_DECIMAL] = KeyNumDelete,
	[VK_DIVIDE] = KeyNumSlash,
	[VK_F1] = KeyF1,
	[VK_F2] = KeyF2,
	[VK_F3] = KeyF3,
	[VK_F4] = KeyF4,
	[VK_F5] = KeyF5,
	[VK_F6] = KeyF6,
	[VK_F7] = KeyF7,
	[VK_F8] = KeyF8,
	[VK_F9] = KeyF9,
	[VK_F10] = KeyF10,
	[VK_F11] = KeyF11,
	[VK_F12] = KeyF12,
	// Windows offers up to F24
	[VK_NUMLOCK] = KeyNumLock,
	[VK_SCROLL] = KeyScrollLock,

	[0xBA] = ';',
	[0xBB] = '=',
	[0xBC] = ',',
	[0xBD] = '-',
	[0xBE] = '.',
	[0xBF] = '/',
	[0xC0] = '`',
	[0xDB] = '[',
	[0xDC] = '\\',
	[0xDD] = ']',
	[0xDE] = '\'',
};

GuiKey_t gui_get_keycode(GuiKeyEvent_t event)
{
	GuiKey_t key;
	if(event.wParam >= 256)
		return 0;
	key = virtual_codes[event.wParam];
	switch(key)
	{
	case KeyControl:
		if((HIWORD(event.lParam) & KF_EXTENDED))
			key = KeyRightControl;
		break;
	case KeyAlt:
		if((HIWORD(event.lParam) & KF_EXTENDED))
			key = KeyRightAlt;
		break;
	// TODO: determine shift key
	}
	return key;
}

GuiPoint_t gui_get_mouse_button_coordinates(GuiMouseButtonEvent_t event)
{
	GuiPoint_t point;
	point.x = LOWORD(event.lParam);
	point.y = HIWORD(event.lParam);
	return point;
}

GuiMouseButton_t gui_get_mouse_buttons(GuiMouseButtonEvent_t event)
{
	GuiMouseButton_t buttons = 0;
	if(event.wParam & MK_LBUTTON)
		buttons |= GUI_MOUSE_BUTTON_LEFT;
	if(event.wParam & MK_RBUTTON)
		buttons |= GUI_MOUSE_BUTTON_RIGHT;
	if(event.wParam & MK_MBUTTON)
		buttons |= GUI_MOUSE_BUTTON_MIDDLE;
	return buttons;
}

GuiMouseButton_t gui_is_double_click(GuiMouseButtonEvent_t event)
{
	return event.double_click;
}

GuiPoint_t gui_get_mouse_move_coordinates(GuiMouseMoveEvent_t event)
{
	GuiPoint_t point;
	point.x = LOWORD(event.lParam);
	point.y = HIWORD(event.lParam);
	return point;
}

GuiRectangle_t gui_window_get_client_area(GuiWindow_t window)
{
	GuiRectangle_t rectangle;
	RECT rect;
	GetClientRect(window, &rect);
	rectangle.x = rect.left;
	rectangle.y = rect.top;
	rectangle.w = rect.right - rect.left + 1;
	rectangle.h = rect.bottom - rect.top + 1;
	return rectangle;
}

void gui_window_redraw(GuiWindow_t window)
{
	InvalidateRect(window, NULL, TRUE);
	UpdateWindow(window);
}

GuiDrawContext_t gui_window_begin_draw(GuiWindow_t window)
{
	GuiDrawContext_t draw_context;
	BeginPaint(window, &draw_context.ps);

	draw_context.hWnd = window;
	draw_context.hdc = GetDC(window);
	draw_context.brush = GetStockObject(BLACK_BRUSH);

#if __I86__ && WINDOWS_TARGET_VERSION < 2
	if(_SetTextAlign)
	{
		_SetTextAlign(draw_context.hdc, TA_BOTTOM);
	}
#else
	SetTextAlign(draw_context.hdc, TA_BOTTOM);
#endif

	return draw_context;
}

void gui_window_end_draw(GuiDrawContext_t * draw_context)
{
	ReleaseDC(draw_context->hWnd, draw_context->hdc);
	EndPaint(draw_context->hWnd, &draw_context->ps);
}

void gui_set_color_black(GuiDrawContext_t * draw_context)
{
	SelectObject(draw_context->hdc, GetStockObject(BLACK_PEN));
	draw_context->brush = GetStockObject(BLACK_BRUSH);
	SetTextColor(draw_context->hdc, RGB(0x00, 0x00, 0x00));
}

void gui_set_color_white(GuiDrawContext_t * draw_context)
{
	SelectObject(draw_context->hdc, GetStockObject(WHITE_PEN));
	draw_context->brush = GetStockObject(WHITE_BRUSH);
	SetTextColor(draw_context->hdc, RGB(0xFF, 0xFF, 0xFF));
}

void gui_fill_rectangle(GuiDrawContext_t * draw_context, int x, int y, int w, int h)
{
	RECT rectangle;
	rectangle.left = x;
	rectangle.top = y;
	rectangle.right = x + w - 1;
	rectangle.bottom = y + h - 1;
	FillRect(draw_context->hdc, &rectangle, draw_context->brush);
}

void gui_draw_line(GuiDrawContext_t * draw_context, int x1, int y1, int x2, int y2)
{
#if __I86__
	MoveTo(draw_context->hdc, x1, y1);
#else
	MoveToEx(draw_context->hdc, x1, y1, (LPPOINT) NULL);
#endif
	LineTo(draw_context->hdc, x2, y2);
}

int gui_get_font_height(GuiDrawContext_t * draw_context)
{
	// TODO
	return 0;
}

void gui_write_text(GuiDrawContext_t * draw_context, int x, int y, const char * text)
{
	int previous_mode = SetBkMode(draw_context->hdc, TRANSPARENT);
	TextOut(draw_context->hdc, x, y, text, strlen(text));
	SetBkMode(draw_context->hdc, previous_mode);
}

HWND * gui_objects = NULL;
size_t gui_objects_count = 0;

static GuiWidget_t gui_register_widget(HWND hWnd)
{
	GuiWidget_t index = gui_objects_count++;
	if(gui_objects)
	{
		gui_objects = realloc(gui_objects, gui_objects_count * sizeof(HWND));
	}
	else
	{
		gui_objects = malloc(gui_objects_count * sizeof(HWND));
	}
	gui_objects[index] = hWnd;
	return index;
}

GuiWidget_t gui_create_root(GuiWindow_t window)
{
	return gui_register_widget(window);
}

GuiWidget_t gui_create_push_button(GuiWindow_t window, GuiWidget_t parent, int x, int y, int w, int h, const char far * caption, long flags)
{
	return gui_register_widget(CreateWindow("BUTTON", caption, WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, x, y, w, h, window, (HMENU) gui_objects_count, NULL, NULL));
}

// Windows graphical applications must have a specific WinMain entry point (at least when compiled with the Watcom compiler)
#if __I86__ || !__NT__
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#endif
{
	GuiMainParameters_t parameters;
	parameters.hInstance = hInstance;
	parameters.hPrevInstance = hPrevInstance;
	parameters.lpCmdLine = lpCmdLine;
	parameters.nCmdShow = nCmdShow;
	return gui_main(parameters);
}

