#ifndef __PFDEFS
#define __PFDEFS

/* Platform specific definitions for IBM OS/2 */

#define INCL_WIN
#define INCL_GPI
#include <os2.h>

typedef HWND GuiWindow_t;
typedef struct
{
	HPS hps;
	LONG window_height;
} GuiDrawContext_t;

typedef CHRMSG GuiKeyEvent_t;

typedef struct
{
	MSEMSG msg;
	GuiMouseButton_t button;
	bool double_click;
} GuiButtonEvent_t;

typedef MSEMSG GuiMouseEvent_t;

typedef struct
{
	int argc;
	char ** argv;
	char ** envp;
} GuiMainParameters_t;

#endif // __PFDEFS
