#ifndef __PFDEFS
#define __PFDEFS

/* Platform specific definitions for Commodore Amiga */

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>

typedef struct Window * GuiWindow_t;

typedef struct { /* TODO */ } GuiDrawContext_t;

typedef struct { /* TODO */ } GuiKeyEvent_t;

typedef struct { /* TODO */ } GuiMouseButtonEvent_t;

typedef struct { /* TODO */ } GuiMouseMoveEvent_t;

typedef struct
{
	int argc;
	char ** argv;
} GuiMainParameters_t;

#endif // __PFDEFS
