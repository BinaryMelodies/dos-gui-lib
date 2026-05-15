
/* Implementation for for DESQview/X, specifically the XCB library */

#include "api.h"
#define _COMPILE_LIBRARY
#include "internal.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define MAX(a, b) ((a) >= (b) ? (a) : (b))

xcb_connection_t * connection;
xcb_screen_t * screen;

xcb_intern_atom_reply_t * WM_PROTOCOLS;
xcb_intern_atom_reply_t * WM_DELETE_WINDOW;

void gui_init(GuiMainParameters_t * parameters)
{
	xcb_intern_atom_cookie_t cookie_WM_PROTOCOLS, cookie_WM_DELETE_WINDOW;

	connection = xcb_connect(NULL, NULL);

	cookie_WM_PROTOCOLS = xcb_intern_atom(connection, 1, sizeof "WM_PROTOCOLS" - 1, "WM_PROTOCOLS");
	WM_PROTOCOLS = xcb_intern_atom_reply(connection, cookie_WM_PROTOCOLS, 0);

	cookie_WM_DELETE_WINDOW = xcb_intern_atom(connection, 0, sizeof "WM_DELETE_WINDOW" - 1, "WM_DELETE_WINDOW");
	WM_DELETE_WINDOW = xcb_intern_atom_reply(connection, cookie_WM_DELETE_WINDOW, 0);

	screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
}

void gui_terminate(void)
{
	xcb_disconnect(connection);
}

// common function, used for both dialog boxes and main windows
static void _init_window(GuiWindow_t window, const char * window_title, int x, int y, int w, int h, uint32_t event_mask)
{
	uint32_t mask = 0;
	uint32_t values[2];

	mask = XCB_CW_BACK_PIXEL;
	values[0] = screen->white_pixel;
	if(event_mask != 0)
	{
		values[1] = event_mask;
		mask |= XCB_CW_EVENT_MASK;
	}

	xcb_create_window(connection, XCB_COPY_FROM_PARENT, window, screen->root, x, y, w, h, 10, XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, mask, values);

	xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, strlen(window_title), window_title);

	xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, WM_PROTOCOLS->atom, XCB_ATOM_ATOM, 32, 1, &WM_DELETE_WINDOW->atom);

	xcb_map_window(connection, window);
}

/* Drawing a dialog box, inspired by https://github.com/Eleobert/MessageBox-X11/blob/master/messagebox.c */

static void get_text_extents(xcb_font_t font, const char * string, int * width, int * height, int * ascent)
{
	xcb_query_text_extents_reply_t * reply;
	xcb_char2b_t * buffer = malloc(strlen(string) * sizeof(xcb_char2b_t));
	int i;

	for(i = 0; string[i] != '\0'; i++)
	{
		buffer[i].byte1 = 0;
		buffer[i].byte2 = string[i];
	}

	reply = xcb_query_text_extents_reply(connection,
		xcb_query_text_extents(connection, font, strlen(string), buffer),
		NULL);

	*width = reply->overall_width;
	*height = reply->font_ascent + reply->font_descent;
	*ascent = reply->font_ascent;

	free(reply);
	free(buffer);
}

void draw_inner_rectangle(xcb_window_t window, xcb_gcontext_t gc, xcb_rectangle_t rectangle, bool selected)
{
	uint32_t values[1];
	values[0] = selected ? screen->black_pixel : screen->white_pixel;
	xcb_change_gc(connection, gc, XCB_GC_FOREGROUND, values);

	rectangle.x += 2;
	rectangle.y += 2;
	rectangle.width -= 4;
	rectangle.height -= 4;
	xcb_poly_rectangle(connection, window, gc, 1, &rectangle);
}

typedef struct
{
	uint32_t flags;
	int32_t x, y;
	int32_t width, height;
	int32_t min_width, min_height;
	int32_t max_width, max_height;
	int32_t width_inc, height_inc;
	int32_t min_aspect_num, min_aspect_den;
	int32_t max_aspect_num, max_aspect_den;
	int32_t base_width, base_height;
	uint32_t win_gravity;
} xcb_size_hints_t;

#define _BUTTON_OK     "OK"
#define _BUTTON_YES    "Yes"
#define _BUTTON_NO     "No"
#define _BUTTON_ABORT  "Abort"
#define _BUTTON_RETRY  "Retry"
#define _BUTTON_IGNORE "Ignore"
#define _BUTTON_CANCEL "Cancel"
#define _BUTTON_HELP   "Help"
#define _MAX_BUTTONS 8

#define _BUTTON_WIDTH 50
#define _TEXT_SKIP 5
#define _BUTTON_SKIP 5

// X11 offers no built-in dialogs, so we have to display our own
int gui_message_box(const char * title, const char * message, GuiMessageBoxButtonSet_t buttons, int default_button, GuiMessageBoxIcon_t icon)
{
	int button_count;
	int button_focus, escape_button;
	bool enter_pressed, button_clicked;
	bool button_selected;
	xcb_window_t window;
	xcb_gcontext_t gc;
	xcb_font_t font;
	int button_mapping[_MAX_BUTTONS];
	xcb_rectangle_t button_rectangles[_MAX_BUTTONS];
	const char * button_messages[_MAX_BUTTONS];
	int buttons_top;
	int text_height, text_ascent;
	int message_width;	
	int window_width;
	int window_height;

	// get the font
	font = xcb_generate_id(connection);
	xcb_open_font(connection, font, sizeof "7x13" - 1, "7x13");

	button_focus = -1;
	escape_button = -1;
	button_count = 0;

	get_text_extents(font, message, &message_width, &text_height, &text_ascent);

	{
		int buttons_left, buttons_right;
		int i;

		buttons_top = text_height + 10;
		buttons_right = 0;

#define _MAKE_BUTTON(__button) \
	{ \
		int text_width; \
		button_messages[button_count] = _BUTTON_##__button; \
		get_text_extents(font, button_messages[button_count], &text_width, &text_height, &text_ascent); \
		button_rectangles[button_count].x = buttons_right; \
		button_rectangles[button_count].y = buttons_top; \
		button_rectangles[button_count].width = text_width + 2 * _TEXT_SKIP; \
		button_rectangles[button_count].height = text_height + 2 * _TEXT_SKIP; \
		buttons_right += text_width + 2 * _TEXT_SKIP + _BUTTON_SKIP; \
		button_mapping[button_count] = GUI_MSGBOX_BUTTON_##__button; \
		button_count ++; \
	}

#define _CHECK_BUTTON(__button) \
	if((buttons & GUI_MSGBOX_BUTTON(__button))) \
	{ \
		if(GUI_MSGBOX_BUTTON_##__button == GUI_MSGBOX_BUTTON_CANCEL) \
			escape_button = button_count; \
		if(default_button == GUI_MSGBOX_BUTTON_##__button) \
			button_focus = button_count; \
		_MAKE_BUTTON(__button); \
	}

		_CHECK_BUTTON(OK);
		_CHECK_BUTTON(YES);
		_CHECK_BUTTON(NO);
		_CHECK_BUTTON(ABORT);
		_CHECK_BUTTON(RETRY);
		_CHECK_BUTTON(IGNORE);
		_CHECK_BUTTON(CANCEL);
		_CHECK_BUTTON(HELP);

		if(button_count == 0)
		{
			_MAKE_BUTTON(OK);
		}

		window_width = MAX(buttons_right, message_width + 2 * _TEXT_SKIP);

		buttons_left = (window_width - buttons_right) / 2;

		for(i = 0; i < button_count; i++)
		{
			button_rectangles[i].x += buttons_left;
		}
	}

	window_height = text_height * 2 + 5 * _TEXT_SKIP;

	window = xcb_generate_id(connection);
	_init_window(window, title,
		(screen->width_in_pixels - window_width) / 2,
		(screen->height_in_pixels - window_height) / 2,
		window_width, window_height, XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE);

	{
		xcb_size_hints_t hints;
#define XCB_ICCCM_SIZE_HINT_P_SIZE (1 << 3)
#define XCB_ICCCM_SIZE_HINT_P_MIN_SIZE (1 << 4)
#define XCB_ICCCM_SIZE_HINT_P_MAX_SIZE (1 << 5)
		hints.flags = XCB_ICCCM_SIZE_HINT_P_SIZE | XCB_ICCCM_SIZE_HINT_P_MIN_SIZE | XCB_ICCCM_SIZE_HINT_P_MAX_SIZE;
		hints.width = window_width;
		hints.height = window_height;
		hints.min_width = window_width;
		hints.min_height = window_height;
		hints.max_width = window_width;
		hints.max_height = window_height;
		xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, XCB_ATOM_WM_NORMAL_HINTS, XCB_ATOM_WM_SIZE_HINTS, 32, sizeof(xcb_size_hints_t) >> 2, &hints);
	}

	// create gc
	gc = xcb_generate_id(connection);
	{
		uint32_t values[3];
		values[0] = screen->black_pixel;
		values[1] = screen->white_pixel;
		values[2] = font;
		xcb_create_gc(connection, gc, window, XCB_GC_FOREGROUND | XCB_GC_BACKGROUND | XCB_GC_FONT, values);
	}
	xcb_close_font(connection, font);

	enter_pressed = false;
	button_clicked = false;
	button_selected = false;
	xcb_flush(connection);
	while(!button_selected)
	{
		xcb_generic_event_t * event;
		while((event = xcb_poll_for_event(connection)))
		{
			switch(event->response_type & ~0x80)
			{
			case XCB_EXPOSE:
				{
					int i;
					uint32_t values[1];
					values[0] = screen->black_pixel;
					xcb_change_gc(connection, gc, XCB_GC_FOREGROUND, values);

					xcb_image_text_8(connection, strlen(message), window, gc, (window_width - message_width) / 2, text_ascent + _TEXT_SKIP, message);
					for(i = 0; i < button_count; i++)
					{
						xcb_poly_rectangle(connection, window, gc, 1, &button_rectangles[i]);
						xcb_image_text_8(connection, strlen(button_messages[i]), window, gc, button_rectangles[i].x + _TEXT_SKIP, buttons_top + text_ascent + _TEXT_SKIP, button_messages[i]);
					}

					if(button_focus >= 0)
						draw_inner_rectangle(window, gc, button_rectangles[button_focus], true);

					xcb_flush(connection);
				}
				break;
			case XCB_KEY_PRESS:
				{
					xcb_key_press_event_t * key_event = (xcb_key_press_event_t *)event;
					switch(key_event->detail)
					{
					case 8: // escape
						if(escape_button != -1)
						{
							if(button_focus >= 0)
								draw_inner_rectangle(window, gc, button_rectangles[button_focus], false);

							button_focus = escape_button;

							draw_inner_rectangle(window, gc, button_rectangles[button_focus], true);
							xcb_flush(connection);

							button_selected = true;
						}
						break;
					case 22: // tab
						if((key_event->state & 1)) // shift
							goto previous_button;
						else
							goto next_button;
					case 113: // left
					case 115: // up
					previous_button:
						if(!enter_pressed && !button_clicked)
						{
							if(button_focus >= 0)
								draw_inner_rectangle(window, gc, button_rectangles[button_focus], false);

							button_focus --;
							if(button_focus < 0)
								button_focus = button_count - 1;

							draw_inner_rectangle(window, gc, button_rectangles[button_focus], true);
							xcb_flush(connection);
						}
						break;
					case 114: // right
					case 116: // down
					next_button:
						if(!enter_pressed && !button_clicked)
						{
							if(button_focus >= 0)
								draw_inner_rectangle(window, gc, button_rectangles[button_focus], false);

							button_focus ++;
							if(button_focus >= button_count)
								button_focus = 0;

							draw_inner_rectangle(window, gc, button_rectangles[button_focus], true);
							xcb_flush(connection);
						}
						break;
					case 35: // enter
						if(!button_clicked)
						{
							if(button_focus >= 0)
							{
								enter_pressed = true;
							}
						}
						break;
					}
				}
				break;
			case XCB_KEY_RELEASE:
				{
					xcb_key_release_event_t * key_event = (xcb_key_release_event_t *)event;
					switch(key_event->detail)
					{
					case 35: // enter
						if(enter_pressed)
						{
							button_selected = true;
						}
						break;
					}
				}
				break;
			case XCB_BUTTON_PRESS:
				{
					if(!enter_pressed && !button_clicked)
					{
						xcb_button_press_event_t * button_event = (xcb_button_press_event_t *)event;

						if(button_event->event_y >= button_rectangles[0].y && button_event->event_y < button_rectangles[0].y + button_rectangles[0].height)
						{
							int i;
							for(i = 0; i < button_count; i++)
							{
								if(button_event->event_x >= button_rectangles[i].x && button_event->event_x < button_rectangles[i].x + button_rectangles[0].width)
								{
									button_clicked = true;

									if(button_focus >= 0)
										draw_inner_rectangle(window, gc, button_rectangles[button_focus], false);

									button_focus = i;

									draw_inner_rectangle(window, gc, button_rectangles[button_focus], true);
									xcb_flush(connection);

									break;
								}
							}
						}
					}
				}
				break;
			case XCB_BUTTON_RELEASE:
				if(button_clicked)
				{
					button_selected = true;
				}
				break;
			case XCB_CLIENT_MESSAGE:
				// ignore
				break;
			}
		}
		free(event);
	}

	xcb_free_gc(connection, gc);
	gui_window_destroy(window);

	return button_focus;
}

GuiWindow_t gui_window_create(const char * window_title, int x, int y, int w, int h)
{
	xcb_window_t window;
	uint32_t event_mask = 0;

	window = xcb_generate_id(connection);

	// set up event mask
	if(callback_show != 0)
		event_mask |= XCB_EVENT_MASK_EXPOSURE;
	if(callback_key_press != 0 || callback_text != 0)
		event_mask |= XCB_EVENT_MASK_KEY_PRESS;
	if(callback_key_release != 0 || callback_text != 0)
		event_mask |= XCB_EVENT_MASK_KEY_RELEASE;
	if(callback_mouse_button_press != 0)
		event_mask |= XCB_EVENT_MASK_BUTTON_PRESS;
	if(callback_mouse_button_release != 0)
		event_mask |= XCB_EVENT_MASK_BUTTON_RELEASE;
	if(callback_mouse_move != 0)
		event_mask |= XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_BUTTON_MOTION;

	if(x == GUI_WINPOS_DEFAULT)
		x = 0; // TODO
	else if(x == GUI_WINPOS_MAXIMUM)
		x = 0; // TODO

	if(y == GUI_WINPOS_DEFAULT)
		y = 0; // TODO
	else if(y == GUI_WINPOS_MAXIMUM)
		y = 0; // TODO

	if(w == GUI_WINPOS_DEFAULT)
		w = 0; // TODO
	else if(w == GUI_WINPOS_MAXIMUM)
		w = 0; // TODO: set to screen width

	if(h == GUI_WINPOS_DEFAULT)
		h = 0; // TODO
	else if(h == GUI_WINPOS_MAXIMUM)
		h = 0; // TODO: set to screen height

	_init_window(window, window_title, x, y, w, h, event_mask);

	return window;
}

static void _window_redraw(GuiWindow_t window)
{
	if(callback_show)
	{
		if(callback_show(window))
		{
			xcb_flush(connection);
		}
	}
}

// X11 keyboard events are always low level events, so we translate (TODO: could we use XIM?)
static bool kbd_left_shift = false;
static bool kbd_right_shift = false;
static bool kbd_caps_lock = false;
static bool kbd_num_lock = false;
static const char kbd_shifted[256] =
{
	['\''] = '"',
	[KeyAsterisk] = '*',
	[KeyPlus] = '+',
	[','] = '<',
	['-'] = '-',
	['.'] = '>',
	['/'] = '?',
	['0'] = ')',
	['1'] = '!',
	['2'] = '@',
	['3'] = '#',
	['4'] = '$',
	['5'] = '%',
	['6'] = '^',
	['7'] = '&',
	['8'] = '*',
	['9'] = '(',
	[';'] = ':',
	['='] = '+',
	['['] = '{',
	['\\'] = '|',
	[']'] = '}',
	['`'] = '~',
};

// controls the gui_main_loop
static bool gui_running;
// the next two fields make a rough simulation of a double click
static xcb_timestamp_t gui_last_button_click;
static xcb_button_t gui_last_button = -1;

int gui_main_loop(void)
{
	gui_running = true;
	xcb_flush(connection);
	while(gui_running)
	{
		xcb_generic_event_t * event;
		while((event = xcb_poll_for_event(connection)))
		{
			switch(event->response_type & ~0x80)
			{
			case XCB_EXPOSE:
				_window_redraw(((xcb_expose_event_t *)event)->window);
				break;
			case XCB_KEY_PRESS:
				if(callback_key_press)
				{
					callback_key_press(((xcb_key_press_event_t *)event)->event, *(xcb_key_press_event_t *)event);
				}

				if(callback_text)
				{
					char c = 0;
					GuiKey_t key = gui_get_keycode(*(xcb_key_press_event_t *)event);
					switch(key)
					{
					//case KeyBackspace:
					case KeyDelete:
						if((kbd_left_shift || kbd_right_shift) ^ kbd_num_lock)
						{
							c = '.';
						}
						break;
					case KeyTab:
						c = '\t';
						break;
					case KeyEnter:
						c = '\r';
						break;
					case KeyLeftShift:
						kbd_left_shift = true;
						break;
					case KeyRightShift:
						kbd_right_shift = true;
						break;
					case KeyCapsLock:
						kbd_caps_lock = true;
						break;
					case KeyNumLock:
						kbd_num_lock = true;
						break;
					case KeyNumMinus:
						c = '-';
						break;
					case KeyNumSlash:
						c = '/';
						break;
					default:
						if('a' <= key && key <= 'z')
						{
							if((kbd_left_shift || kbd_right_shift) ^ kbd_caps_lock)
							{
								c = key - 'a' + 'A';
							}
							else
							{
								c = key;
							}
						}
						else if(' ' <= key && key <= '`')
						{
							if(kbd_left_shift || kbd_right_shift)
							{
								c = kbd_shifted[key];
							}
							else
							{
								c = key;
							}
						}
						else if(KeyNum0 <= key && key <= KeyNum9)
						{
							if((kbd_left_shift || kbd_right_shift) ^ kbd_num_lock)
							{
								c = key - KeyNum0 + '0';
							}
						}
						break;
					}

					if(c != 0)
					{
						callback_text(((xcb_key_press_event_t *)event)->event, 1, &c);
						xcb_flush(connection);
					}
				}
				else if(callback_key_press)
				{
					xcb_flush(connection);
				}
				break;
			case XCB_KEY_RELEASE:
				if(callback_key_release)
				{
					callback_key_release(((xcb_key_release_event_t *)event)->event, *(xcb_key_press_event_t *)event);
					xcb_flush(connection);
				}

				if(callback_text)
				{
					char c = 0;
					GuiKey_t key = gui_get_keycode(*(xcb_key_press_event_t *)event);
					switch(key)
					{
					case KeyLeftShift:
						kbd_left_shift = false;
						break;
					case KeyRightShift:
						kbd_right_shift = false;
						break;
					case KeyCapsLock:
						kbd_caps_lock = false;
						break;
					case KeyNumLock:
						kbd_num_lock = false;
						break;
					}
				}
				break;
			case XCB_BUTTON_PRESS:
				if(callback_mouse_button_press)
				{
					GuiMouseButtonEvent_t button_event;
					button_event.event = *(xcb_button_press_event_t *)event;
					button_event.double_click =
						gui_last_button == ((xcb_button_press_event_t *)event)->detail
						&& ((xcb_button_press_event_t *)event)->time - gui_last_button_click < 200;

					gui_last_button_click = ((xcb_button_press_event_t *)event)->time;
					gui_last_button = ((xcb_button_press_event_t *)event)->detail;

					callback_mouse_button_press(((xcb_button_press_event_t *)event)->event, button_event);
					xcb_flush(connection);
				}
				break;
			case XCB_BUTTON_RELEASE:
				if(callback_mouse_button_release)
				{
					GuiMouseButtonEvent_t button_event;
					button_event.event = *(xcb_button_press_event_t *)event;
					button_event.double_click = false;

					callback_mouse_button_release(((xcb_button_release_event_t *)event)->event, button_event);
					xcb_flush(connection);
				}
				break;
			case XCB_MOTION_NOTIFY:
				if(callback_mouse_move)
				{
					callback_mouse_move(((xcb_button_release_event_t *)event)->event, *(xcb_motion_notify_event_t *)event);
					xcb_flush(connection);
				}
				break;
			case XCB_CLIENT_MESSAGE:
				if(((xcb_client_message_event_t *)event)->data.data32[0] == WM_DELETE_WINDOW->atom)
				{
					if(callback_quit)
					{
						bool override = callback_quit(((xcb_client_message_event_t *)event)->window);
						xcb_flush(connection);
						if(override)
							break;
					}
					gui_terminate_main_loop();
				}
				break;
			}
		}
		free(event);
	}
	return 0;
}

void gui_terminate_main_loop(void)
{
	gui_running = false;
}

void gui_window_destroy(GuiWindow_t window)
{
	xcb_destroy_window(connection, window);
}

static const GuiKey_t keycodes[256] =
{
	[8] = KeyEscape,
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', KeyBackspace,
	KeyTab, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', KeyEnter,
	KeyControl, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
	KeyShift, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', KeyRightShift,
	'*',
	0, // (63)
	' ', KeyCapsLock,
	KeyF1, KeyF2, KeyF3, KeyF4, KeyF5, KeyF6, KeyF7, KeyF8, KeyF9, KeyF10,
	KeyNumLock, KeyScrollLock,
	KeyNum7, KeyNum8, KeyNum9, KeyNumMinus,
	KeyNum4, KeyNum5, KeyNum6, '+',
	KeyNum1, KeyNum2, KeyNum3, KeyNum0,
	KeyNumDelete,
	0, // (91)
	0, // (92)
	0, // (93)
	KeyF11, KeyF12,
	/* when the NumLock key is on */
	KeyNum7, KeyNum8, KeyNum9, KeyNumMinus,
	KeyNum4, KeyNum5, KeyNum6, '+',
	KeyNum1, KeyNum2, KeyNum3, KeyNum0,
	KeyNumDelete,
	KeyRightControl,
	KeyNumEnter,
	KeyNumSlash,
	0, // (112)
	KeyLeft,
	KeyRight,
	KeyUp,
	KeyDown,
	KeyInsert,
	KeyDelete,
	KeyHome,
	KeyEnd,
	KeyPageUp,
	KeyPageDown,
	KeyPause,
};

GuiKey_t gui_get_keycode(GuiKeyEvent_t event)
{
	return keycodes[event.detail];
}

GuiPoint_t gui_get_mouse_button_coordinates(GuiMouseButtonEvent_t event)
{
	GuiPoint_t point;
	point.x = event.event.event_x;
	point.y = event.event.event_y;
	return point;
}

GuiMouseButton_t gui_get_mouse_buttons(GuiMouseButtonEvent_t event)
{
	GuiMouseButton_t buttons = 0;
	switch(event.event.detail)
	{
	case 1:
		buttons = GUI_MOUSE_BUTTON_LEFT;
		break;
	case 2:
		buttons = GUI_MOUSE_BUTTON_MIDDLE;
		break;
	case 3:
		buttons = GUI_MOUSE_BUTTON_RIGHT;
		break;
	}
	return buttons;
}

GuiMouseButton_t gui_is_double_click(GuiMouseButtonEvent_t event)
{
	return event.double_click;
}

GuiPoint_t gui_get_mouse_move_coordinates(GuiMouseMoveEvent_t event)
{
	GuiPoint_t point;
	point.x = event.event_x;
	point.y = event.event_y;
	return point;
}

GuiRectangle_t gui_window_get_client_area(GuiWindow_t window)
{
	GuiRectangle_t rectangle;
	xcb_get_geometry_reply_t * reply;

	reply = xcb_get_geometry_reply(connection,
		xcb_get_geometry(connection, window),
		NULL);

	rectangle.x = reply->x;
	rectangle.y = reply->y;
	rectangle.w = reply->width;
	rectangle.h = reply->height;

	free(reply);

	return rectangle;
}

void gui_window_redraw(GuiWindow_t window)
{
	/*GuiRectangle_t rectangle;

	rectangle = gui_window_get_client_area(window);
	xcb_clear_area(connection, 1, window, rectangle.x, rectangle.y, rectangle.w, rectangle.h);*/

	_window_redraw(window);
}

GuiDrawContext_t gui_window_begin_draw(GuiWindow_t window)
{
	GuiDrawContext_t draw_context;
	uint32_t values[3];

	draw_context.window = window;

	draw_context.current_font = xcb_generate_id(connection);
	xcb_open_font(connection, draw_context.current_font, sizeof "7x13" - 1, "7x13");

	// create GC

	draw_context.graphics_context = xcb_generate_id(connection);
	values[0] = screen->black_pixel;
	values[1] = screen->white_pixel;
	values[2] = draw_context.current_font;
	xcb_create_gc(connection, draw_context.graphics_context, screen->root, XCB_GC_FOREGROUND | XCB_GC_BACKGROUND | XCB_GC_FONT, values);

	return draw_context;
}

void gui_window_end_draw(GuiDrawContext_t * draw_context)
{
	xcb_close_font(connection, draw_context->current_font);
	xcb_free_gc(connection, draw_context->graphics_context);
}

void gui_set_color_black(GuiDrawContext_t * draw_context)
{
	uint32_t values[1];
	values[0] = screen->black_pixel;
	xcb_change_gc(connection, draw_context->graphics_context, XCB_GC_FOREGROUND, values);
}

void gui_set_color_white(GuiDrawContext_t * draw_context)
{
	uint32_t values[1];
	values[0] = screen->white_pixel;
	xcb_change_gc(connection, draw_context->graphics_context, XCB_GC_FOREGROUND, values);
}

void gui_fill_rectangle(GuiDrawContext_t * draw_context, int x, int y, int w, int h)
{
	xcb_rectangle_t rectangles[1];
//printf("[%d,%d,%d,%d]\n", x, y, w, h);
	rectangles[0].x = x;
	rectangles[0].y = y;
	rectangles[0].width = w;
	rectangles[0].height = h;
	xcb_poly_fill_rectangle(connection, draw_context->window, draw_context->graphics_context, 1, rectangles);
}

void gui_draw_line(GuiDrawContext_t * draw_context, int x1, int y1, int x2, int y2)
{
	xcb_point_t points[2];
	points[0].x = x1;
	points[0].y = y1;
	points[1].x = x2;
	points[1].y = y2;
	xcb_poly_line(connection, XCB_COORD_MODE_ORIGIN, draw_context->window, draw_context->graphics_context, 2, points);
}

int gui_get_font_height(GuiDrawContext_t * draw_context)
{
	int height;
	xcb_query_text_extents_reply_t * reply;

	reply = xcb_query_text_extents_reply(connection,
		xcb_query_text_extents(connection, draw_context->current_font, 0, NULL),
		NULL);

	height = reply->font_ascent + reply->font_descent;

	free(reply);

	return height;

}

void gui_write_text(GuiDrawContext_t * draw_context, int x, int y, const char * text)
{
	char * item = malloc(strlen(text) + 2);
	item[0] = strlen(text);
	item[1] = 0;
	memcpy(&item[2], text, strlen(text));
	xcb_poly_text_8(connection, draw_context->window, draw_context->graphics_context, x, y, strlen(text) + 2, item);
	free(item);
}

GuiWidget_t gui_create_root(GuiWindow_t window)
{
	return 0; // TODO
}

GuiWidget_t gui_create_push_button(GuiWindow_t window, GuiWidget_t parent, int x, int y, int w, int h, const char far * caption, long flags)
{
	return 0; // TODO
}

int main(int argc, char ** argv, char ** envp)
{
	GuiMainParameters_t parameters;
	parameters.argc = argc;
	parameters.argv = argv;
	parameters.envp = envp;
	return gui_main(parameters);
}

