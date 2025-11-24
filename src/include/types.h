#ifndef __TYPES_H
#define __TYPES_H

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
	GUI_BUTTON_OK = 1,
	GUI_BUTTON_YES,
	GUI_BUTTON_NO,
	GUI_BUTTON_ABORT,
	GUI_BUTTON_RETRY,
	GUI_BUTTON_IGNORE,
	GUI_BUTTON_CANCEL,
	GUI_BUTTON_HELP,
} GuiMessageBoxButton_t;
#define GUI_BUTTON(__button) (1 << (GUI_BUTTON_##__button))

/* Constants for message box icons */
typedef enum
{
	GUI_ICON_NONE,
	GUI_ICON_WARNING,
	GUI_ICON_QUESTION,
	GUI_ICON_STOP,
	GUI_ICON_INFORMATION,
} GuiMessageBoxIcon_t;

/* Constants for mouse buttons */
typedef enum
{
	GUI_BUTTON_LEFT = 0x01,
	GUI_BUTTON_RIGHT = 0x02,
	GUI_BUTTON_MIDDLE = 0x04,

	GUI_SINGLE_CLICK = 0x10,
	GUI_DOUBLE_CLICK = 0x20,
} GuiMouseButton_t;

#endif // __TYPES_H
