
/* Implementation for Digital Research GEM */

#include "api.h"
#include "internal.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>

#undef YES
#undef NO

WORD appl;
WORD vdi_handle;

WORD char_width;
WORD char_height;
WORD box_width;
WORD box_height;

WORD workstation_input[11];
WORD workstation_output[57];

void gui_init(GuiMainParameters_t * parameters)
{
	int i;

	appl = appl_init(NULL);
	vdi_handle = graf_handle(&char_width, &char_height, &box_width, &box_height);
	for(i = 0; i < 10; i++)
		workstation_input[i] = 1;
	workstation_input[i] = 2;
	v_opnvwk(workstation_input, &vdi_handle, workstation_output);
}

void gui_terminate(void)
{
	// TODO
}

static bool gui_last_button_state = false;
static bool gui_running;

static bool gui_mouse_position_known = false;
static GuiPoint_t gui_last_mouse_position;

int gui_main_loop(void)
{
	WORD msg[8];

	gui_running = true;
	while(gui_running)
	{
		UWORD keycode, keystate;
		UWORD mouse_x, mouse_y, mouse_buttons, click_count;
		WORD x, y;
		WORD which;

		if(callback_mousemove != NULL)
		{
			if(!gui_mouse_position_known)
			{
				//wind_get(window, WF_WXYWH, &x, &y, &w, &h);
				x = 0;
				y = 0;
			}
			else
			{
				x = gui_last_mouse_position.x;
				y = gui_last_mouse_position.y;
			}
		}

		which = evnt_multi(MU_MESAG
				| (callback_keypress ? MU_KEYBD : 0)
				| (callback_buttonpress || callback_buttonrelease ? MU_BUTTON : 0)
				| (callback_mousemove ? MU_M1 : 0),
			// button
			callback_clickcount_mask & GUI_DOUBLE_CLICK ? 2 : 1,
				// we can only check for a single mouse button
			callback_buttons_mask & GUI_BUTTON_LEFT ? 1 : callback_buttons_mask & GUI_BUTTON_RIGHT ? 2 : 4,
			gui_last_button_state ? 0 : callback_buttons_mask & GUI_BUTTON_LEFT ? 1 : callback_buttons_mask & GUI_BUTTON_RIGHT ? 2 : 4,
			// mouse
			1, x, y, 1, 1,
			0, 0, 0, 0, 0,
			// mesag
			msg,
			// timer
			0, 0,
			// mouse
			&mouse_x, &mouse_y, &mouse_buttons,
			// keybd
			&keystate, &keycode, &click_count);

		if(which & MU_MESAG)
		{
			switch(msg[0])
			{
#if 0
			case AC_OPEN:
				break;
			case AC_CLOSE:
				gui_running = false;
				break;
#endif
			case WM_REDRAW:
				gui_window_redraw(msg[3]);
				break;
			case WM_TOPPED:
				wind_set(msg[3], WF_TOP, 0, 0, 0, 0);
				break;
			case WM_CLOSED:
				if(callback_quit && callback_quit(msg[3]))
					break;

				gui_window_destroy(msg[3]);
				gui_terminate_main_loop();
				break;
			case WM_MOVED:
			case WM_SIZED:
				wind_set(msg[3], WF_CXYWH, msg[4], msg[5], msg[6], msg[7]);
				gui_window_redraw(msg[3]);
				break;
			}
		}

		if((which & MU_KEYBD) != 0)
		{
			// TODO: key releases are not supported
			if(callback_keypress)
			{
				GuiKeyEvent_t event;
				event.keycode = keycode;
				event.keystate = keystate;
				callback_keypress(msg[3], event);
			}
			if(callback_text)
			{
				char c = keycode & 0xFF;
				if(c != 0)
				{
					callback_text(msg[3], 1, &c);
				}
			}
		}

		if((which & (MU_BUTTON | MU_M1)) != 0)
		{
			gui_mouse_position_known = true;
			gui_last_mouse_position.x = mouse_x;
			gui_last_mouse_position.y = mouse_y;
		}

		if((which & MU_BUTTON) != 0)
		{
			GuiButtonEvent_t event;
			event.mouse_x = mouse_x;
			event.mouse_y = mouse_y;
			//event.mouse_buttons = mouse_buttons; // TODO: does not work
			event.mouse_buttons = callback_buttons_mask & GUI_BUTTON_LEFT ? GUI_BUTTON_LEFT : callback_buttons_mask & GUI_BUTTON_RIGHT ? GUI_BUTTON_RIGHT : GUI_BUTTON_MIDDLE;
			event.click_count = click_count;
			event.keystate = keystate;
			if(gui_last_button_state)
			{
				if(callback_buttonrelease)
				{
					callback_buttonrelease(msg[3], event);
				}
			}
			else
			{
				if(callback_buttonpress)
				{
					if((click_count == 1 && (callback_clickcount_mask & GUI_SINGLE_CLICK) != 0)
					|| (click_count == 2 && (callback_clickcount_mask & GUI_DOUBLE_CLICK) != 0))
					{
						callback_buttonpress(msg[3], event);
					}
				}
			}
			gui_last_button_state ^= true;
		}

		if((which & MU_M1) != 0 && callback_mousemove)
		{
			GuiMouseEvent_t event;

			event.mouse_x = mouse_x;
			event.mouse_y = mouse_y;
			event.mouse_buttons = mouse_buttons; // TODO: does not work
			//event.mouse_buttons = callback_buttons_mask & GUI_BUTTON_LEFT ? GUI_BUTTON_LEFT : callback_buttons_mask & GUI_BUTTON_RIGHT ? GUI_BUTTON_RIGHT : GUI_BUTTON_MIDDLE;
			event.keystate = keystate;

			callback_mousemove(msg[3], event);
		}
	}

	return 0;
}

void gui_terminate_main_loop(void)
{
	gui_running = false;
}

#define _BUTTON_OK     " OK "
#define _BUTTON_YES    " Yes "
#define _BUTTON_NO     " No "
#define _BUTTON_ABORT  " Abort "
#define _BUTTON_RETRY  " Retry "
#define _BUTTON_IGNORE " Ignore "
#define _BUTTON_CANCEL " Cancel "
#define _BUTTON_HELP   " Help "
#define _MAX_BUTTONS 8
#define _MAX_BUTTONS_TEMPLATE (sizeof _BUTTON_OK _BUTTON_YES _BUTTON_NO _BUTTON_ABORT _BUTTON_RETRY _BUTTON_IGNORE _BUTTON_CANCEL _BUTTON_HELP + 2 + _MAX_BUTTONS)

int gui_message_box(const char * title, const char * message, int buttons, int default_button, GuiMessageBoxIcon_t icon)
{
	char buttons_template[_MAX_BUTTONS_TEMPLATE];
	int button_mapping[_MAX_BUTTONS+1];
	int button_count;
	int default_value;
	char * buffer;
	int length;
	int result;
	int icon_number;

	buttons_template[0] = '\0';
	button_count = 0;
	default_value = 0;
	memset(button_mapping, -1, sizeof button_mapping);

#define _CHECK_BUTTON(__button) \
	if((buttons & GUI_BUTTON(__button)) && button_count < 3) \
	{ \
		if(buttons_template[0] == 0) \
			strcpy(buttons_template, "["); \
		else \
			strcat(buttons_template, "|"); \
		strcat(buttons_template, _BUTTON_##__button); \
		button_mapping[button_count ++] = GUI_BUTTON_##__button; \
		if(default_button == GUI_BUTTON_##__button) \
			default_value = button_count; \
	}

	_CHECK_BUTTON(OK);
	_CHECK_BUTTON(YES);
	_CHECK_BUTTON(NO);
	_CHECK_BUTTON(ABORT);
	_CHECK_BUTTON(RETRY);
	_CHECK_BUTTON(IGNORE);
	_CHECK_BUTTON(CANCEL);
	_CHECK_BUTTON(HELP);

	if(buttons_template[0] == 0)
		strcpy(buttons_template, "[" _BUTTON_OK "]"); // have at least 1 button
	else
		strcat(buttons_template, "]");

	switch(icon)
	{
	case GUI_ICON_NONE:
	case GUI_ICON_INFORMATION:
		icon_number = 0;
		break;
	case GUI_ICON_WARNING:
		icon_number = 1;
		break;
	case GUI_ICON_QUESTION:
		icon_number = 2;
		break;
	case GUI_ICON_STOP:
		icon_number = 3;
		break;
	}

	if(title != NULL)
		length = snprintf(NULL, 0, "[%d][%s|%s]%s", icon_number, title, message, buttons_template);
	else
		length = snprintf(NULL, 0, "[%d][%s]%s", icon_number, message, buttons_template);
	buffer = malloc(length + 1);
	if(title != NULL)
		snprintf(buffer, length + 1, "[%d][%s|%s]%s", icon_number, title, message, buttons_template);
	else
		snprintf(buffer, length + 1, "[%d][%s]%s", icon_number, message, buttons_template);

	result = form_alert(default_value, buffer);

	free(buffer);

	return button_mapping[result];
}

GuiWindow_t gui_window_create(const char * window_title, int x, int y, int w, int h)
{
	WORD window;

	graf_mouse(M_OFF, 0L);
	window = wind_create(NAME | CLOSER | MOVER | SIZER, x, y, w, h);
	wind_set(window, WF_NAME, FP_OFF(window_title), FP_SEG(window_title), 0, 0);
	wind_open(window, x, y, w, h);
	graf_mouse(M_ON, 0L);

	return window;
}

void gui_window_destroy(GuiWindow_t window)
{
	graf_mouse(M_OFF, 0L);
	wind_close(window);
	wind_delete(window);
	graf_mouse(M_ON, 0L);
}

static const GuiKey_t keycodes[256] =
{
	0,
	KeyEscape,
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', KeyBackspace,
	KeyTab, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', KeyEnter,
	0, // (29)
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
	0, // (42)
	'\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', KeyRightShift,
	'*',
	0, // (56)
	' ',
	0, // (58)
	KeyF1, KeyF2, KeyF3, KeyF4, KeyF5, KeyF6, KeyF7, KeyF8, KeyF9, KeyF10,
	0, // (61)
	[71] = KeyHome, KeyUp, KeyNum9, KeyNumMinus,
	KeyLeft, KeyNum5, KeyRight, '+',
	KeyEnd, KeyDown, KeyNum3, KeyInsert,
	KeyDelete,
};

GuiKey_t gui_get_keycode(GuiKeyEvent_t event)
{
	return keycodes[event.keycode >> 8];
}

GuiPoint_t gui_get_button_coordinates(GuiButtonEvent_t event)
{
	GuiPoint_t point;
	point.x = event.mouse_x;
	point.y = event.mouse_y;
	return point;
}

GuiMouseButton_t gui_get_buttons(GuiButtonEvent_t event)
{
	GuiMouseButton_t buttons = 0;
	if(event.mouse_buttons & 1)
		buttons |= GUI_BUTTON_LEFT;
	if(event.mouse_buttons & 2)
		buttons |= GUI_BUTTON_RIGHT;
	if(event.mouse_buttons & 4)
		buttons |= GUI_BUTTON_MIDDLE;
	return buttons;
}

GuiMouseButton_t gui_is_double_click(GuiButtonEvent_t event)
{
	return event.click_count >= 2;
}

GuiPoint_t gui_get_mouse_coordinates(GuiMouseEvent_t event)
{
	GuiPoint_t point;
	point.x = event.mouse_x;
	point.y = event.mouse_y;
	return point;
}

GuiRectangle_t gui_window_get_client_area(GuiWindow_t window)
{
	GuiRectangle_t rectangle;
	WORD x, y, w, h;
	wind_get(window, WF_WXYWH, &x, &y, &w, &h);
	rectangle.x = x;
	rectangle.y = y;
	rectangle.w = w;
	rectangle.h = h;
	return rectangle;
}

void gui_window_redraw(GuiWindow_t window)
{
	if(callback_show)
	{
		callback_show(window);
	}
}

GuiDrawContext_t gui_window_begin_draw(GuiWindow_t window)
{
	GuiRectangle_t client_area = gui_window_get_client_area(window);
	WORD rect[4];

	wind_update(BEG_UPDATE);
	graf_mouse(M_OFF, 0L);

	rect[0] = client_area.x;
	rect[1] = client_area.y;
	rect[2] = client_area.x + client_area.w - 1;
	rect[3] = client_area.y + client_area.h - 1;
	vs_clip(vdi_handle, TRUE, rect);

	return window;
}

void gui_window_end_draw(GuiDrawContext_t draw_context)
{
	GuiRectangle_t client_area = gui_window_get_client_area(draw_context);
	WORD rect[4];
	rect[0] = client_area.x;
	rect[1] = client_area.y;
	rect[2] = client_area.x + client_area.w - 1;
	rect[3] = client_area.y + client_area.h - 1;
	vs_clip(vdi_handle, FALSE, rect);

	graf_mouse(M_ON, 0L);
	wind_update(END_UPDATE);
}

void gui_set_color_black(GuiDrawContext_t draw_context)
{
	vsf_color(vdi_handle, BLACK);
	vsl_color(vdi_handle, BLACK);
	vst_color(vdi_handle, BLACK);
}

void gui_set_color_white(GuiDrawContext_t draw_context)
{
	vsf_color(vdi_handle, WHITE);
	vsl_color(vdi_handle, WHITE);
	vst_color(vdi_handle, WHITE);
}

void gui_fill_rectangle(GuiDrawContext_t draw_context, int x, int y, int w, int h)
{
	WORD xys[4];
	GuiRectangle_t client_area = gui_window_get_client_area(draw_context);
	xys[0] = client_area.x + x;
	xys[1] = client_area.y + y;
	xys[2] = client_area.x + x + w - 1;
	xys[3] = client_area.y + y + w - 1;
	vswr_mode(vdi_handle, MD_REPLACE);
	vr_recfl(vdi_handle, xys);
}

void gui_draw_line(GuiDrawContext_t draw_context, int x1, int y1, int x2, int y2)
{
	WORD xys[4];
	GuiRectangle_t client_area = gui_window_get_client_area(draw_context);
	xys[0] = client_area.x + x1;
	xys[1] = client_area.y + y1;
	xys[2] = client_area.x + x2;
	xys[3] = client_area.y + y2;
	vswr_mode(vdi_handle, MD_REPLACE);
	v_pline(vdi_handle, 2, xys);
}

int gui_get_font_height(GuiDrawContext_t draw_context)
{
	// TODO
	return 0;
}

void gui_write_text(GuiDrawContext_t draw_context, int x, int y, const char * text)
{
	GuiRectangle_t client_area = gui_window_get_client_area(draw_context);
	vswr_mode(vdi_handle, MD_TRANS);
	v_gtext(vdi_handle, client_area.x + x, client_area.y + y, (char *)text);
//	vst_color(vdi_handle, WHITE);
//	v_gtext(vdi_handle, 10, 10, "HHHEY");
}

int main(int argc, char ** argv, char ** envp)
{
	GuiMainParameters_t parameters;
	parameters.argc = argc;
	parameters.argv = argv;
	parameters.envp = envp;
	return gui_main(parameters);
}

