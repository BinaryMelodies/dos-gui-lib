#ifndef __PFDEFS
#define __PFDEFS

/* Platform specific definitions for Digital Research GEM */

#include <gembind.h>

typedef WORD GuiWindow_t;

typedef GuiWindow_t GuiDrawContext_t;

typedef struct
{
	WORD keycode;
	WORD keystate;
} GuiKeyEvent_t;

typedef struct
{
	UWORD mouse_x, mouse_y, mouse_buttons, click_count, keystate;
} GuiMouseButtonEvent_t;

typedef struct
{
	UWORD mouse_x, mouse_y/*, mouse_buttons*/, keystate;
} GuiMouseMoveEvent_t;

typedef struct
{
	int argc;
	char ** argv;
	char ** envp;
} GuiMainParameters_t;

#endif // __PFDEFS
