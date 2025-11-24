#ifndef __PFDEFS
#define __PFDEFS

/* Platform specific definitions for DESQview/X, specifically the XCB library */

#include <xcb/xcb.h>

typedef xcb_window_t GuiWindow_t;
typedef struct
{
	xcb_window_t window;
	xcb_gcontext_t graphics_context;
	xcb_font_t current_font;
	//int current_font_height;
} GuiDrawContext_t;

typedef xcb_key_press_event_t GuiKeyEvent_t;

typedef struct
{
	xcb_button_press_event_t event;
	bool double_click;
} GuiMouseButtonEvent_t;

typedef xcb_motion_notify_event_t GuiMouseMoveEvent_t;

typedef struct
{
	int argc;
	char ** argv;
	char ** envp;
} GuiMainParameters_t;

#endif // __PFDEFS
