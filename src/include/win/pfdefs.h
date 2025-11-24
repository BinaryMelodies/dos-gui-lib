#ifndef __PFDEFS
#define __PFDEFS

/* Platform specific definitions for Microsoft Windows */

#include <windows.h>

#define main _c_main

typedef HWND GuiWindow_t;
typedef struct {
	HWND hWnd;
	PAINTSTRUCT ps;
	HDC hdc;
	HBRUSH brush;
} GuiDrawContext_t;

typedef struct
{
	WPARAM wParam;
	LPARAM lParam;
} GuiKeyEvent_t;

typedef struct
{
	WPARAM wParam;
	LPARAM lParam;
	bool double_click;
} GuiMouseButtonEvent_t, GuiMouseMoveEvent_t;

typedef struct
{
	HINSTANCE hInstance;
	HINSTANCE hPrevInstance;
	LPSTR lpCmdLine;
	int nCmdShow;
} GuiMainParameters_t;

#endif // __PFDEFS
