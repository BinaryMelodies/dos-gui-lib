#ifndef __PFDEFS
#define __PFDEFS

/* Platform specific definitions for Apple Macintosh */

#include <Windows.h>

typedef WindowRef GuiWindow_t;

typedef struct
{
	WindowRef window;
	GrafPtr savedPort;
	INTEGER font;
} GuiDrawContext_t;

typedef struct EventRecord GuiKeyEvent_t;

typedef struct EventRecord GuiMouseButtonEvent_t;

typedef struct EventRecord GuiMouseMoveEvent_t;

typedef struct { } GuiMainParameters_t;

#endif // __PFDEFS
