
/* Implementation for Digital Research GEM */

#include "api.h"
#define _COMPILE_LIBRARY
#include "internal.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <dos.h>

#undef YES
#undef NO

/* widget handling */

struct gui_widgets
{
	GuiWindow_t window;
	OBJECT far * objects;
	size_t objects_count;
	// main loop context
	WORD edit_obj;
	WORD next_obj;
	WORD index;
};

struct gui_widgets * gui_widgets;
size_t gui_widgets_count;

static void gui_window_create_widgets(GuiWindow_t window)
{
	size_t index = gui_widgets_count++;
	if(gui_widgets)
	{
		gui_widgets = realloc(gui_widgets, gui_widgets_count * sizeof(struct gui_widgets));
	}
	else
	{
		gui_widgets = malloc(gui_widgets_count * sizeof(struct gui_widgets));
	}
	gui_widgets[index].window = window;
	gui_widgets[index].objects = NULL;
	gui_widgets[index].objects_count = 0;
}

static struct gui_widgets * gui_obtain_widgets(GuiWindow_t window)
{
	size_t index;
	for(index = 0; index < gui_widgets_count; index ++)
	{
		if(gui_widgets[index].window == window)
			return &gui_widgets[index];
	}
	return NULL;
}

static void gui_window_dispose_widgets(struct gui_widgets * widgets)
{
	size_t index = widgets - gui_widgets;
	if(index != gui_widgets_count - 1)
	{
		memcpy(&gui_widgets[index], &gui_widgets[gui_widgets_count - 1], sizeof(struct gui_widgets));
	}

	if(--gui_widgets_count > 0)
	{
		gui_widgets = realloc(gui_widgets, gui_widgets_count * sizeof(struct gui_widgets));
	}
	else
	{
		free(gui_widgets);
		gui_widgets = NULL;
	}
}

static void gui_widgets_redraw_part(WORD window, OBJECT far * tree, WORD cl_x, WORD cl_y, WORD cl_w, WORD cl_h);
static void gui_widgets_redraw(WORD window, OBJECT far * tree);

static WORD gui_append_object(struct gui_widgets * widgets, UWORD ob_type, UWORD ob_flags, UWORD ob_state, void far * ob_spec, UWORD ob_x, UWORD ob_y, UWORD ob_width, UWORD ob_height)
{
	WORD index = widgets->objects_count++;
	if(widgets->objects)
	{
		widgets->objects = _frealloc(widgets->objects, widgets->objects_count * sizeof(OBJECT));
	}
	else
	{
		widgets->objects = _fmalloc(widgets->objects_count * sizeof(OBJECT));
	}
	widgets->objects[index].ob_next = NIL;
	widgets->objects[index].ob_head = NIL;
	widgets->objects[index].ob_tail = NIL;
	widgets->objects[index].ob_type = ob_type;
	widgets->objects[index].ob_flags = ob_flags | LASTOB;
	widgets->objects[index].ob_state = ob_state;
	widgets->objects[index].ob_spec = ob_spec;
	widgets->objects[index].ob_x = ob_x;
	widgets->objects[index].ob_y = ob_y;
	widgets->objects[index].ob_width = ob_width;
	widgets->objects[index].ob_height = ob_height;
	if(index != 0)
	{
		widgets->objects[index - 1].ob_flags &= ~LASTOB;
	}
	return index;
}

static WORD objc_get_parent(OBJECT far * tree, WORD object)
{
	while(true)
	{
		WORD next_object = tree[object].ob_next;
		if(next_object < ROOT || tree[next_object].ob_tail == object)
			return next_object;
		object = next_object;
	}
}

static void objc_get_absolute_position(OBJECT far * tree, WORD object, WORD * xp, WORD * yp)
{
	WORD x, y;
	x = 0;
	y = 0;
	while(object != NIL)
	{
		x += tree[object].ob_x;
		y += tree[object].ob_y;
		object = objc_get_parent(tree, object);
	}
	*xp = x;
	*yp = y;
}

// based on form_do
static WORD objc_find_following(OBJECT far * tree, WORD start_obj, WORD end_obj, WORD direction, WORD check_flag)
{
	WORD obj;

	if(direction == FMD_BACKWARD)
	{
		direction = -1;
	}
	else
	{
		direction = 1;
	}

	obj = start_obj + direction;

	while(obj >= 0 && obj != end_obj)
	{
		WORD objflags = tree[obj].ob_flags;
		WORD objstate = tree[obj].ob_state;
		if(!(objstate & DISABLED) && !(objflags & HIDETREE))
		{
			if(objflags & check_flag)
			{
				return obj;
			}
		}

		if(objflags & LASTOB)
		{
			return NIL;
		}
		else
		{
			obj += direction;
		}
	}

	return NIL;
}

static WORD objc_find_next(OBJECT far * tree, WORD start_obj, WORD direction, WORD check_flag)
{
	WORD obj;

	if(direction == FMD_BACKWARD)
	{
		obj = objc_find_following(tree, start_obj, NIL, FMD_BACKWARD, check_flag);
		if(obj != NIL)
			return obj;
		obj = objc_find_following(tree, start_obj, NIL, FMD_FORWARD, LASTOB);
		obj = objc_find_following(tree, obj + 1, start_obj, FMD_BACKWARD, check_flag);
		if(obj == NIL)
			return start_obj;
		else
			return obj;
	}
	else
	{
		if(!(tree[start_obj].ob_flags & LASTOB))
		{
			obj = objc_find_following(tree, start_obj, NIL, FMD_FORWARD, check_flag);
			if(obj != NIL)
				return obj;
		}
		obj = objc_find_following(tree, ROOT, start_obj, FMD_FORWARD, check_flag);
		if(obj == NIL)
			return start_obj;
		else
			return obj;
	}
}

static void objc_set_highlighted(WORD window, OBJECT far * tree, WORD object, bool highlight)
{
	WORD x, y;

	if(object == 0 || !(tree[object].ob_flags & (SELECTABLE | EDITABLE)))
		return;

	if(highlight)
	{
		if(!(tree[object].ob_state & HIGHLIGHTED))
		{
			objc_get_absolute_position(tree, object, &x, &y);
#if 0
			objc_change(tree, object, 0, x - 5, y - 5, tree[object].ob_width + 10, tree[object].ob_height + 10, tree[object].ob_state | HIGHLIGHTED, true);
#else
			tree[object].ob_state |= HIGHLIGHTED;
			objc_draw(tree, object, 0, x - 5, y - 5, tree[object].ob_width + 10, tree[object].ob_height + 10);
#endif
		}
	}
	else
	{
		if(tree[object].ob_state & HIGHLIGHTED)
		{
			objc_get_absolute_position(tree, object, &x, &y);
#if 0
			/* TODO: this does not seem to remove the highlighting */
			objc_change(tree, object, 0, x - 5, y - 5, tree[object].ob_width + 10, tree[object].ob_height + 10, tree[object].ob_state & ~HIGHLIGHTED, true);
#else
			objc_draw(tree, object, 0, x - 5, y - 5, tree[object].ob_width + 10, tree[object].ob_height + 10); // the graphics routine XORs the highlight
			tree[object].ob_state &= ~HIGHLIGHTED;
			/*objc_draw(tree, object, 0, x - 5, y - 5, tree[object].ob_width + 10, tree[object].ob_height + 10);*/
			//gui_widgets_redraw_part(window, tree, x - 5, y - 5, tree[object].ob_width + 10, tree[object].ob_height + 10); // TODO: does not redraw the background
#endif
		}
	}
}

/* actual program data */

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

static bool gui_click_action(struct gui_widgets * widgets, WORD object, WORD clicks, WORD * pnext)
{
	bool to_continue = form_button(widgets->objects, object, clicks, pnext);
	if(callback_action && !gui_last_button_state)
		callback_action(widgets->window, object, GUI_ACTION_CLICKED);
	return to_continue;
}

// parts of it based on form_do
int gui_main_loop(void)
{
	WORD msg[8];
	struct gui_widgets * widgets = NULL;

	/* set initial widgets */
	{
		size_t index;
		for(index = 0; index < gui_widgets_count; index ++)
		{
			gui_widgets[index].next_obj = objc_find_next(gui_widgets[index].objects, ROOT, FMD_FORWARD, DEFAULT);
			gui_widgets[index].edit_obj = 0;
		}
	}

	gui_running = true;
	while(gui_running)
	{
		UWORD keycode, keystate;
		UWORD mouse_x, mouse_y, mouse_buttons, click_count;
		WORD x, y;
		WORD which;

		if(callback_mouse_move != NULL)
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

		/* select currently active window */
		{
			WORD active, discard;
			wind_get(0, WF_TOP, &active, &discard, &discard, &discard);
			widgets = gui_obtain_widgets(active);
		}

		/* select new widget to gain focus */
		if((widgets->next_obj != 0) && (widgets->edit_obj != widgets->next_obj))
		{
			widgets->edit_obj = widgets->next_obj;
			widgets->next_obj = 0;
			objc_set_highlighted(widgets->window, widgets->objects, widgets->edit_obj, true);
			if(widgets->objects[widgets->edit_obj].ob_flags & EDITABLE)
			{
				objc_edit(widgets->objects, widgets->edit_obj, 0, &widgets->index, EDINIT);
			}
		}

		{
			UWORD events = MU_MESAG;
			UWORD check_click_count;
			UWORD check_button_mask;
			UWORD check_button_state;

			if(callback_key_press)
				events |= MU_KEYBD;
			if(callback_mouse_button_press || callback_mouse_button_release)
				events |= MU_BUTTON;
			if(callback_mouse_move)
				events |= MU_M1;
			if(widgets->objects_count > 0)
				events |= MU_KEYBD | MU_BUTTON;

			if(callback_click_count_mask & GUI_MOUSE_CLICK_DOUBLE)
				check_click_count = 2;
			else
				check_click_count = 1;

			// we can only check for a single mouse button
			if(widgets->objects_count > 0)
				check_button_mask = 1;
			else if(callback_mouse_buttons_mask & GUI_MOUSE_BUTTON_LEFT)
				check_button_mask = 1;
			else if(callback_mouse_buttons_mask & GUI_MOUSE_BUTTON_RIGHT)
				check_button_mask = 2;
			else
				check_button_mask = 4;

			if(gui_last_button_state)
				check_button_state = 0;
			else
				check_button_state = check_button_mask;

			which = evnt_multi(events,
				// button
				check_click_count, check_button_mask, check_button_state,
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
		}

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
				objc_set_highlighted(widgets->window, widgets->objects, widgets->edit_obj, false);
				wind_set(msg[3], WF_TOP, 0, 0, 0, 0);
				{
					WORD active, discard;
					wind_get(0, WF_TOP, &active, &discard, &discard, &discard);
					widgets = gui_obtain_widgets(active);
					objc_set_highlighted(widgets->window, widgets->objects, widgets->edit_obj, true);
				}
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
			if(callback_key_press)
			{
				GuiKeyEvent_t event;
				event.keycode = keycode;
				event.keystate = keystate;
				callback_key_press(msg[3], event);
			}

			if(callback_text)
			{
				char c = keycode & 0xFF;
				if(c != 0)
				{
					callback_text(msg[3], 1, &c);
				}
			}

			/* widget handling */
			switch(keycode)
			{
			case 0x1C0D: // RETURN
				if(widgets->objects[widgets->edit_obj].ob_flags & EXIT)
				{
					widgets->next_obj = widgets->edit_obj;
				}
				else
				{
					widgets->next_obj = objc_find_following(widgets->objects, widgets->edit_obj - 1, NIL, FMD_FORWARD, DEFAULT);
					if(widgets->next_obj == NIL)
						widgets->next_obj = widgets->edit_obj;
				}

				if(widgets->objects[widgets->next_obj].ob_flags & EXIT)
				{
					WORD discard;
					if(widgets->next_obj != widgets->edit_obj)
						gui_running &= gui_click_action(widgets, widgets->edit_obj, 1, &discard);
					widgets->edit_obj = widgets->next_obj;
					gui_running &= gui_click_action(widgets, widgets->next_obj, 1, &widgets->next_obj);
				}
				break;
			case 0x3920: // SPACE
				widgets->next_obj = widgets->edit_obj;
				objc_set_highlighted(widgets->window, widgets->objects, widgets->edit_obj, false);
				gui_running &= gui_click_action(widgets, widgets->next_obj, 1, &widgets->next_obj);
				objc_set_highlighted(widgets->window, widgets->objects, widgets->edit_obj, true);
				break;
			case 0x4B00: // LEFT
				if(widgets->objects[widgets->edit_obj].ob_flags & EDITABLE && widgets->index != 0)
				{
					gui_running &= form_keybd(widgets->objects, widgets->edit_obj, widgets->next_obj, keycode, &widgets->next_obj, (WORD *)&keycode);
					break;
				}
				// fallthru
			case 0x0F00: // BACK TAB
				widgets->next_obj = objc_find_next(widgets->objects, widgets->edit_obj, FMD_BACKWARD, EDITABLE | SELECTABLE);
				break;
			case 0x4800: // UP
				if(widgets->objects[widgets->edit_obj].ob_flags & RBUTTON)
				{
					WORD parent_obj;
					parent_obj = objc_get_parent(widgets->objects, widgets->edit_obj);
					widgets->next_obj = objc_find_following(widgets->objects, widgets->objects[parent_obj].ob_head, NIL, FMD_BACKWARD, EDITABLE | SELECTABLE);
					if(widgets->next_obj == NIL)
						widgets->next_obj = widgets->objects[parent_obj].ob_head;
					parent_obj = objc_get_parent(widgets->objects, widgets->next_obj);
					widgets->next_obj = objc_find_following(widgets->objects, widgets->objects[parent_obj].ob_head, NIL, FMD_FORWARD, EDITABLE | SELECTABLE);
					if(widgets->next_obj == NIL)
						widgets->next_obj = widgets->objects[parent_obj].ob_head;
				}
				else
				{
					widgets->next_obj = widgets->edit_obj;
				}
				widgets->next_obj = objc_find_next(widgets->objects, widgets->next_obj, FMD_BACKWARD, EDITABLE | SELECTABLE);
				break;
			case 0x4D00: // RIGHT
				if((widgets->objects[widgets->edit_obj].ob_flags & EDITABLE) && widgets->index < _fstrlen(((TEDINFO far *)widgets->objects[widgets->edit_obj].ob_spec)->te_ptext))
				{
					gui_running &= form_keybd(widgets->objects, widgets->edit_obj, widgets->next_obj, keycode, &widgets->next_obj, (WORD *)&keycode);
					break;
				}
				// fallthru
			case 0x0F09: // TAB
				widgets->next_obj = objc_find_next(widgets->objects, widgets->edit_obj, FMD_FORWARD, EDITABLE | SELECTABLE);
				break;
			case 0x5000: // DOWN
				if(widgets->objects[widgets->edit_obj].ob_flags & RBUTTON)
				{
					WORD parent_obj = objc_get_parent(widgets->objects, widgets->edit_obj);
					widgets->next_obj = objc_find_following(widgets->objects, widgets->objects[parent_obj].ob_tail, NIL, FMD_FORWARD, EDITABLE | SELECTABLE);
					if(widgets->next_obj == NIL)
						widgets->next_obj = widgets->objects[parent_obj].ob_tail;
				}
				else
				{
					widgets->next_obj = objc_find_next(widgets->objects, widgets->edit_obj, FMD_FORWARD, EDITABLE | SELECTABLE);
				}
				break;
			case 0x011B: // ESC
				{
					WORD esc_obj = objc_find_following(widgets->objects, widgets->edit_obj, NIL, FMD_FORWARD, ESCCANCEL);
					if(esc_obj == NIL)
						widgets->next_obj = widgets->edit_obj;
					if(widgets->objects[esc_obj].ob_flags & ESCCANCEL)
					{
						objc_set_highlighted(widgets->window, widgets->objects, widgets->edit_obj, false);
						widgets->next_obj = widgets->edit_obj = esc_obj;
						gui_running &= gui_click_action(widgets, widgets->next_obj, 1, &widgets->next_obj);
					}
				}
				break;
			/* TODO: does not seem to work */
			case 0x4900: // P_UP (page up)
				widgets->next_obj = objc_find_following(widgets->objects, ROOT, NIL, FMD_FORWARD, SCROLLER);
				if(widgets->next_obj == NIL)
					widgets->next_obj = ROOT;
				gui_running &= gui_click_action(widgets, widgets->next_obj, 1, &widgets->next_obj);
				break;
			/* TODO: does not seem to work */
			case 0x5100: // P_DOWN (page down)
				widgets->next_obj = objc_find_following(widgets->objects, widgets->next_obj, NIL, FMD_FORWARD, LASTOB);
				{
					WORD last_obj = widgets->next_obj;
					widgets->next_obj = objc_find_following(widgets->objects, widgets->next_obj, NIL, FMD_BACKWARD, SCROLLER);
					if(widgets->next_obj == NIL)
						widgets->next_obj = last_obj;
				}
				gui_running &= gui_click_action(widgets, widgets->next_obj, 1, &widgets->next_obj);
				break;
			default:
				// TODO: handle shortcuts
				gui_running &= form_keybd(widgets->objects, widgets->edit_obj, widgets->next_obj, keycode, &widgets->next_obj, (WORD *)&keycode);
				break;
			}

			if(keycode)
			{
				if(widgets->objects[widgets->edit_obj].ob_flags & EDITABLE)
				{
					objc_edit(widgets->objects, widgets->edit_obj, keycode, &widgets->index, EDCHAR);
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
			GuiMouseButtonEvent_t event;
			event.mouse_x = mouse_x;
			event.mouse_y = mouse_y;
			//event.mouse_buttons = mouse_buttons; // TODO: does not work
			event.mouse_buttons = callback_mouse_buttons_mask & GUI_MOUSE_BUTTON_LEFT ? GUI_MOUSE_BUTTON_LEFT : callback_mouse_buttons_mask & GUI_MOUSE_BUTTON_RIGHT ? GUI_MOUSE_BUTTON_RIGHT : GUI_MOUSE_BUTTON_MIDDLE;
			event.click_count = click_count;
			event.keystate = keystate;
			if(gui_last_button_state)
			{
				if(callback_mouse_button_release)
				{
					callback_mouse_button_release(msg[3], event);
				}
			}
			else
			{
				if(callback_mouse_button_press)
				{
					if((click_count == 1 && (callback_click_count_mask & GUI_MOUSE_CLICK_SINGLE) != 0)
					|| (click_count == 2 && (callback_click_count_mask & GUI_MOUSE_CLICK_DOUBLE) != 0))
					{
						callback_mouse_button_press(msg[3], event);
					}
				}
			}

			/* widget handling */
			widgets->next_obj = objc_find(widgets->objects, ROOT, MAX_DEPTH, mouse_x, mouse_y);
			if(widgets->next_obj == NIL)
			{
				widgets->next_obj = 0;
			}
			else
			{
				gui_running &= gui_click_action(widgets, widgets->next_obj, click_count, &widgets->next_obj);
			}

			gui_last_button_state ^= true;
		}

		if((which & MU_M1) != 0 && callback_mouse_move)
		{
			GuiMouseMoveEvent_t event;

			event.mouse_x = mouse_x;
			event.mouse_y = mouse_y;
			//event.mouse_buttons = mouse_buttons; // TODO: does not work
			//event.mouse_buttons = callback_buttons_mask & GUI_MOUSE_BUTTON_LEFT ? GUI_MOUSE_BUTTON_LEFT : callback_buttons_mask & GUI_MOUSE_BUTTON_RIGHT ? GUI_MOUSE_BUTTON_RIGHT : GUI_MOUSE_BUTTON_MIDDLE;
			event.keystate = keystate;

			callback_mouse_move(msg[3], event);
		}

		if(!gui_running || ((widgets->next_obj != 0) && (widgets->next_obj != widgets->edit_obj)))
		{
			objc_set_highlighted(widgets->window, widgets->objects, widgets->edit_obj, false);
			if(widgets->objects[widgets->edit_obj].ob_flags & EDITABLE)
			{
				objc_edit(widgets->objects, widgets->edit_obj, 0, &widgets->index, EDEND);
			}
		}
	}

	objc_set_highlighted(widgets->window, widgets->objects, widgets->edit_obj, false);
	objc_set_highlighted(widgets->window, widgets->objects, widgets->next_obj, false);

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

int gui_message_box(const char * title, const char * message, GuiMessageBoxButtonSet_t buttons, int default_button, GuiMessageBoxIcon_t icon)
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
	if((buttons & GUI_MSGBOX_BUTTON(__button)) && button_count < 3) \
	{ \
		if(buttons_template[0] == 0) \
			strcpy(buttons_template, "["); \
		else \
			strcat(buttons_template, "|"); \
		strcat(buttons_template, _BUTTON_##__button); \
		button_mapping[button_count ++] = GUI_MSGBOX_BUTTON_##__button; \
		if(default_button == GUI_MSGBOX_BUTTON_##__button) \
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
	case GUI_MSGBOX_ICON_NONE:
	case GUI_MSGBOX_ICON_INFORMATION:
		icon_number = 0;
		break;
	case GUI_MSGBOX_ICON_WARNING:
		icon_number = 1;
		break;
	case GUI_MSGBOX_ICON_QUESTION:
		icon_number = 2;
		break;
	case GUI_MSGBOX_ICON_STOP:
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

	graf_mouse(M_OFF, 0L);
	window = wind_create(NAME | CLOSER | MOVER | SIZER, x, y, w, h);
	wind_set(window, WF_NAME, FP_OFF(window_title), FP_SEG(window_title), 0, 0);
	wind_open(window, x, y, w, h);
	graf_mouse(M_ON, 0L);

	gui_window_create_widgets(window);

	return window;
}

void gui_window_destroy(GuiWindow_t window)
{
	graf_mouse(M_OFF, 0L);
	wind_close(window);
	wind_delete(window);
	graf_mouse(M_ON, 0L);

	gui_window_dispose_widgets(gui_obtain_widgets(window));
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

GuiPoint_t gui_get_mouse_button_coordinates(GuiMouseButtonEvent_t event)
{
	GuiPoint_t point;
	point.x = event.mouse_x;
	point.y = event.mouse_y;
	return point;
}

GuiMouseButton_t gui_get_mouse_buttons(GuiMouseButtonEvent_t event)
{
	GuiMouseButton_t buttons = 0;
	if(event.mouse_buttons & 1)
		buttons |= GUI_MOUSE_BUTTON_LEFT;
	if(event.mouse_buttons & 2)
		buttons |= GUI_MOUSE_BUTTON_RIGHT;
	if(event.mouse_buttons & 4)
		buttons |= GUI_MOUSE_BUTTON_MIDDLE;
	return buttons;
}

GuiMouseButton_t gui_is_double_click(GuiMouseButtonEvent_t event)
{
	return event.click_count >= 2;
}

GuiPoint_t gui_get_mouse_move_coordinates(GuiMouseMoveEvent_t event)
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

static void gui_widgets_redraw_part(WORD window, OBJECT far * tree, WORD cl_x, WORD cl_y, WORD cl_w, WORD cl_h)
{
	WORD x, y, w, h;
	WORD object;

	// restrict the clipping area to the inside of the window
	wind_get(window, WF_WXYWH, &x, &y, &w, &h);
	if(cl_x + cl_w > x + w)
	{
		cl_w = w + x - cl_x;
		if(cl_w < 0)
			return;
	}
	if(cl_y + cl_h > y + h)
	{
		cl_h = h + y - cl_y;
		if(cl_h < 0)
			return;
	}
	if(cl_x < x)
	{
		cl_w -= x - cl_x;
		cl_x = x;
	}
	if(cl_y < y)
	{
		cl_h -= y - cl_y;
		cl_y = y;
	}

#if 0
	objc_draw(tree, ROOT, MAX_DEPTH, cl_x, cl_y, cl_w, cl_h);
#endif

	wind_update(BEG_UPDATE);
	graf_mouse(M_OFF, 0L);

#if 1
	// do not draw the top level object, only its children
	for(object = tree[ROOT].ob_head; object != NIL && object != ROOT; object = tree[object].ob_next)
	{
		objc_draw(tree, object, MAX_DEPTH, cl_x, cl_y, cl_w, cl_h);
	}
#endif

	graf_mouse(M_ON, 0L);
	wind_update(END_UPDATE);
}

static void gui_widgets_redraw(WORD window, OBJECT far * tree)
{
	WORD x, y, w, h;
	wind_get(window, WF_WXYWH, &x, &y, &w, &h);
	tree[ROOT].ob_x = x;
	tree[ROOT].ob_y = y;
	tree[ROOT].ob_width = w;
	tree[ROOT].ob_height = h;
	gui_widgets_redraw_part(window, tree, x, y, w, h);
}

void gui_window_redraw(GuiWindow_t window)
{
	if(callback_show)
	{
		callback_show(window);
	}
	gui_widgets_redraw(window, gui_obtain_widgets(window)->objects);
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

void gui_window_end_draw(GuiDrawContext_t * draw_context)
{
	GuiRectangle_t client_area = gui_window_get_client_area(*draw_context);
	WORD rect[4];
	rect[0] = client_area.x;
	rect[1] = client_area.y;
	rect[2] = client_area.x + client_area.w - 1;
	rect[3] = client_area.y + client_area.h - 1;
	vs_clip(vdi_handle, FALSE, rect);

	graf_mouse(M_ON, 0L);
	wind_update(END_UPDATE);
}

void gui_set_color_black(GuiDrawContext_t * draw_context)
{
	vsf_color(vdi_handle, BLACK);
	vsl_color(vdi_handle, BLACK);
	vst_color(vdi_handle, BLACK);
}

void gui_set_color_white(GuiDrawContext_t * draw_context)
{
	vsf_color(vdi_handle, WHITE);
	vsl_color(vdi_handle, WHITE);
	vst_color(vdi_handle, WHITE);
}

void gui_fill_rectangle(GuiDrawContext_t * draw_context, int x, int y, int w, int h)
{
	WORD xys[4];
	GuiRectangle_t client_area = gui_window_get_client_area(*draw_context);
	xys[0] = client_area.x + x;
	xys[1] = client_area.y + y;
	xys[2] = client_area.x + x + w - 1;
	xys[3] = client_area.y + y + w - 1;
	vswr_mode(vdi_handle, MD_REPLACE);
	vr_recfl(vdi_handle, xys);
}

void gui_draw_line(GuiDrawContext_t * draw_context, int x1, int y1, int x2, int y2)
{
	WORD xys[4];
	GuiRectangle_t client_area = gui_window_get_client_area(*draw_context);
	xys[0] = client_area.x + x1;
	xys[1] = client_area.y + y1;
	xys[2] = client_area.x + x2;
	xys[3] = client_area.y + y2;
	vswr_mode(vdi_handle, MD_REPLACE);
	v_pline(vdi_handle, 2, xys);
}

int gui_get_font_height(GuiDrawContext_t * draw_context)
{
	// TODO
	return 0;
}

void gui_write_text(GuiDrawContext_t * draw_context, int x, int y, const char * text)
{
	GuiRectangle_t client_area = gui_window_get_client_area(*draw_context);
	vswr_mode(vdi_handle, MD_TRANS);
	v_gtext(vdi_handle, client_area.x + x, client_area.y + y, (char *)text);
}

static GuiWidget_t gui_obtain_root(struct gui_widgets * widgets)
{
	if(widgets->objects_count == 0)
	{
		WORD gem_flags = NONE;
		WORD state = NONE;
		LONG spec = 0x21100L; // interior color: 0, interior pattern: 0, transparent background, text color: 1, border color: 1, border thickness: 2 (never used)
		GuiRectangle_t rectangle = gui_window_get_client_area(widgets->window);
		gui_append_object(widgets, G_BOX, gem_flags, state, (void far *)spec, rectangle.x, rectangle.y, rectangle.w, rectangle.h);
	}
	return ROOT;
}

GuiWidget_t gui_create_push_button(GuiWindow_t window, GuiWidget_t parent, int x, int y, int w, int h, const char far * caption, long flags)
{
	// TODO: this is a toggleable button, not a push button
	WORD gem_flags = SELECTABLE; // TODO: DEFAULT, EXIT, ESCCANCEL
	WORD state = NONE;
	WORD button;
	struct gui_widgets * widgets = gui_obtain_widgets(window);
	gui_obtain_root(widgets);
	button = gui_append_object(widgets, G_BUTTON, gem_flags, state, (void far *)caption, x, y, w, h);
	objc_add(widgets->objects, parent == GuiWindowRoot ? ROOT : parent, button);
	return button;
}

int main(int argc, char ** argv, char ** envp)
{
	GuiMainParameters_t parameters;
	parameters.argc = argc;
	parameters.argv = argv;
	parameters.envp = envp;
	return gui_main(parameters);
}

