#ifndef __TYPES_H
#define __TYPES_H

/*** Introduces several universal types for all backends ***/

#include <stdbool.h>
#include <stdint.h>

#if __386__
# define far
#endif

/* Virtual key values */
typedef enum
{
	KeyBackspace = '\b',
	KeyTab = '\t',
	KeyEnter = '\r',
	KeyEscape = 27,

	/* The number and symbol keys map to their ASCII values, letters are mapped to lower case values */

	/* Numeric keys */
	KeyAsterisk = '*',
	KeyNumAsterisk = KeyAsterisk,
	KeyPlus = '+',
	KeyNumPlus = KeyPlus,

	/* Control keys */
	KeyLeft = 128,
	KeyRight,
	KeyUp,
	KeyDown,
	KeyPageUp,
	KeyPageDown,
	KeyHome,
	KeyEnd,
	KeyInsert,
	KeyDelete,
	KeyF1,
	KeyF2,
	KeyF3,
	KeyF4,
	KeyF5,
	KeyF6,
	KeyF7,
	KeyF8,
	KeyF9,
	KeyF10,
	KeyF11,
	KeyF12,
	KeyPause,
	KeyPrintScreen,

	/* Numeric pad keys */
	KeyNum0,
	KeyNum1,
	KeyNum2,
	KeyNum3,
	KeyNum4,
	KeyNum5,
	KeyNum6,
	KeyNum7,
	KeyNum8,
	KeyNum9,
	KeyNumDelete,
	KeyNumEnter,
	KeyNumMinus,
	KeyNumSlash,

	/* Modifiers and toggles */
	KeyControl,
	KeyLeftControl = KeyControl,
	KeyRightControl,
	KeyShift,
	KeyLeftShift = KeyShift,
	KeyRightShift,
	KeyAlt,
	KeyLeftAlt = KeyAlt,
	KeyRightAlt,
	KeyCapsLock,
	KeyNumLock,
	KeyScrollLock,
#define KeyF(__n) ((GuiKey_t)((__n) - 1 + KeyF1))
} GuiKey_t;

typedef struct
{
	uint16_t x, y;
} GuiPoint_t;

typedef struct
{
	uint16_t x, y, w, h;
} GuiRectangle_t;

/* Constants for message box buttons */
typedef enum
{
	GUI_MSGBOX_BUTTON_OK = 1,
	GUI_MSGBOX_BUTTON_YES,
	GUI_MSGBOX_BUTTON_NO,
	GUI_MSGBOX_BUTTON_ABORT,
	GUI_MSGBOX_BUTTON_RETRY,
	GUI_MSGBOX_BUTTON_IGNORE,
	GUI_MSGBOX_BUTTON_CANCEL,
	GUI_MSGBOX_BUTTON_HELP,
} GuiMessageBoxButton_t;

typedef uint16_t GuiMessageBoxButtonSet_t;
/* Allows creating a set of buttons */
#define GUI_MSGBOX_BUTTON(__button) ((GuiMessageBoxButtonSet_t)1 << (GUI_MSGBOX_BUTTON_##__button))

/* Constants for message box icons */
typedef enum
{
	GUI_MSGBOX_ICON_NONE,
	GUI_MSGBOX_ICON_WARNING,
	GUI_MSGBOX_ICON_QUESTION,
	GUI_MSGBOX_ICON_STOP,
	GUI_MSGBOX_ICON_INFORMATION,
} GuiMessageBoxIcon_t;

/* Constants for mouse buttons */
typedef enum
{
	GUI_MOUSE_BUTTON_LEFT = 0x01,
	GUI_MOUSE_BUTTON_RIGHT = 0x02,
	GUI_MOUSE_BUTTON_MIDDLE = 0x04,

	GUI_MOUSE_CLICK_SINGLE = 0x10,
	GUI_MOUSE_CLICK_DOUBLE = 0x20,
} GuiMouseButton_t;

/* Constants for window position */
enum
{
	GUI_WINPOS_DEFAULT = -1,
	GUI_WINPOS_MAXIMUM = -2,
};

/* Index counter for widgets */
typedef uint16_t GuiWidget_t;

#endif // __TYPES_H
