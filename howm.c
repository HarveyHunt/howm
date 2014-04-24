#include <xcb/xcb.h>

#include <xcb/xcb_keysyms.h>
#include <xcb/xcb_icccm.h>
#include <X11/X.h>
#include <X11/keysym.h>
#include <err.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#define MOVE_RESIZE_MASK (XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | \
			XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT)
#define CLEANMASK(mask) (mask & ~(numlockmask | XCB_MOD_MASK_LOCK))
#define EQUALMODS(mask, omask) (CLEANMASK(mask) == CLEANMASK(omask))
#define LENGTH(x) (sizeof(x) / sizeof(*x))
#define FFT(client) (c->is_transient || c->is_floating || c->is_fullscreen)

typedef union {
	float f;
	int i;
	const char **cmd;
} Arg;

typedef struct {
	unsigned int mod;
	unsigned int mode;
	xcb_keysym_t sym;
	void (*func)(const Arg *);
	const Arg arg;
} Key;

typedef struct {
	unsigned int mod;
	xcb_keysym_t sym;
	void (*func)(const int type, const int count);
} Operator;

typedef struct {
	unsigned int mod;
	xcb_keysym_t sym;
	unsigned int type;
} Motion;

typedef struct {
	unsigned int mod;
	short int button;
	void (*func)(const Arg *);
	const Arg arg;
} Button;

typedef struct Client {
	struct Client *next;
	short int x, y, w, h;
	bool is_fullscreen, is_floating, is_transient;
	xcb_window_t win;
} Client;

typedef struct {
	int layout;
	Client *head, *prev_foc, *current;
} Workspace;

/* Operators */
static void op_kill(const int type, int count);
static void op_move_up(const int type, int count);
static void op_move_down(const int type, int count);

/* Clients */
static void move_current_down(void);
static void move_current_up(void);
static void kill_client(void);
static void move_down(Client *c);
static void focus_next(void);
static void focus_prev(void);
static void move_up(Client *c);
static void update_focused_client(Client *c);
static Client *prev_client(Client *c);
static Client *client_from_window(xcb_window_t w);
static void remove_from_workspace(Client *c);
static void remove_client(Client *c);
static Client *win_to_client(xcb_window_t w);
static void client_to_workspace(Client *c, const int ws);

/* Workspaces */
static void kill_workspace(const int ws);
static void next_workspace(void);
static void previous_workspace(void);
static void last_workspace(void);
static void change_workspace(const Arg *arg);
static void save_workspace(int i);
static void select_workspace(int i);

/* Layouts */
static void change_layout(const Arg *arg);
static void next_layout(void);
static void previous_layout(void);
static void last_layout(void);
static void stack(void);
static void grid(void);
static void zoom(void);
static void fibonacci(void);
static void arrange_windows(void);

/* Modes */
static void change_mode(const Arg *arg);

/* Events */
static void enter_event(xcb_generic_event_t *ev);
static void destroy_event(xcb_generic_event_t *ev);
static void button_press_event(xcb_generic_event_t *ev);
static void key_press_event(xcb_generic_event_t *ev);
static void map_request_event(xcb_generic_event_t *ev);

/* XCB */
static void grab_keys(void);
static xcb_keycode_t *keysym_to_keycode(xcb_keysym_t sym);
static void grab_keycode(xcb_keycode_t *keycode, const int mod);
static void elevate_window(xcb_window_t win);
static void move_resize(xcb_connection_t *con, xcb_window_t win,
				bool draw_gap, int x, int y, int w, int h);
static void set_border_width(xcb_connection_t *con, xcb_window_t win,
				int w);
static void get_atoms(char **names, xcb_atom_t *atoms, unsigned int cnt);
static void check_other_wm(void);
static xcb_keysym_t keycode_to_keysym(xcb_keycode_t keycode);

/* Misc */
static void howm_info(void);
static int get_non_tff_count(void);
static unsigned int get_colour(char *colour);
static void spawn(const Arg *arg);
static void setup(void);

enum {ZOOM, GRID, HSTACK, VSTACK, FIBONACCI, END_LAYOUT};
enum {OPERATOR_STATE, COUNT_STATE, MOTION_STATE, END_STATE};
enum {NORMAL, FOCUS, RESIZE, END_MODES};
enum {COMMAND, OPERATOR, MOTION, END_TYPE};
enum {CLIENT, WORKSPACE};
enum {NET_WM_STATE_FULLSCREEN, NET_SUPPORTED, NET_WM_STATE,
		NET_ACTIVE_WINDOW};
enum {WM_DELETE_WINDOW, WM_PROTOCOLS};

/* Handlers */
static void (*handler[XCB_NO_OPERATION])(xcb_generic_event_t *) = {
	[XCB_BUTTON_PRESS] = button_press_event,
	[XCB_KEY_PRESS] = key_press_event,
	[XCB_MAP_REQUEST] = map_request_event,
	[XCB_DESTROY_NOTIFY] = destroy_event,
	[XCB_ENTER_NOTIFY] = enter_event
};

static void(*layout_handler[])(void) = {
	[GRID] = grid,
	[ZOOM] = zoom,
	[HSTACK] = stack,
	[VSTACK] = stack,
	[FIBONACCI] = fibonacci
};

static void (*operator_func)(const int type, int count);

static xcb_connection_t *dpy;
static char *WM_ATOM_NAMES[] = {"WM_DELETE_WINDOW", "WM_PROTOCOLS"};
static char *NET_ATOM_NAMES[] = {"_NET_WM_STATE_FULLSCREEN", "_NET_SUPPORTED",
				"_NET_WM_STATE", "_NET_ACTIVE_WINDOW"};
static xcb_atom_t wm_atoms[LENGTH(WM_ATOM_NAMES)],
		  net_atoms[LENGTH(NET_ATOM_NAMES)];
static xcb_screen_t *screen;
static xcb_generic_event_t *ev;
static int numlockmask;
static Client *head, *prev_foc, *current;
/* We don't need the range of unsigned, so this prevents a conversion later. */
static int cur_workspace, prev_workspace, cur_layout, prev_layout;
static unsigned int screen_height, screen_width, border_focus, border_unfocus;
static unsigned int cur_mode, cur_count, cur_state = OPERATOR_STATE;

#include "config.h"

#ifdef DEBUG_ENABLE
#	define DEBUG(x) puts(x);
#	define DEBUGP(x, ...) printf(x, ##__VA_ARGS__);
#else
#	define DEBUG(x) do {} while (0)
#	define DEBUGP(x, ...) do {} while (0)
#endif

void setup(void)
{
	screen = xcb_setup_roots_iterator(xcb_get_setup(dpy)).data;
	if (screen == NULL)
		err(EXIT_FAILURE, "Can't acquire the default screen\n");
	screen_height = screen->height_in_pixels;
	screen_width = screen->width_in_pixels;

	DEBUGP("Screen's height is: %d\n", screen_height);
	DEBUGP("Screen's width is: %d\n", screen_width);

	grab_keys();

	get_atoms(NET_ATOM_NAMES, net_atoms, LENGTH(NET_ATOM_NAMES));
	get_atoms(WM_ATOM_NAMES, wm_atoms, LENGTH(WM_ATOM_NAMES));

	border_focus = get_colour(BORDER_FOCUS);
	border_unfocus = get_colour(BORDER_UNFOCUS);
}

unsigned int get_colour(char *colour)
{
	unsigned int r, g, b, pixel;
	xcb_alloc_color_reply_t *rep;
	xcb_colormap_t map = screen->default_colormap;

	char hex[3][3] = {
		{colour[1], colour[2], '\0'},
		{colour[3], colour[4], '\0'},
		{colour[5], colour[6], '\0'} };
	unsigned int rgb16[3] = {(strtol(hex[0], NULL, 16)),
				(strtol(hex[1], NULL, 16)),
				(strtol(hex[2], NULL, 16))};

	r = rgb16[0], g = rgb16[1], b = rgb16[2];
	rep = xcb_alloc_color_reply(dpy, xcb_alloc_color(dpy, map,
					r * 257, g*257, b * 257), NULL);
	if (!rep)
		err(EXIT_FAILURE, "ERROR: Can't allocate the colour %s\n",
				colour);
	pixel = rep->pixel;
	free(rep);
	return pixel;
}

int main(int argc, char *argv[])
{
	dpy = xcb_connect(NULL, NULL);
	if (xcb_connection_has_error(dpy))
		err(EXIT_FAILURE, "Can't open XCB connection\n");
	setup();
	check_other_wm();
	while (!xcb_connection_has_error(dpy)) {
		xcb_flush(dpy);
		ev = xcb_wait_for_event(dpy);
		if (handler[ev->response_type])
			handler[ev->response_type](ev);
	}
	return 1;
}

void check_other_wm(void)
{
	xcb_generic_error_t *e;
	unsigned int values[1] = {XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
				XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
				XCB_EVENT_MASK_BUTTON_PRESS |
				XCB_EVENT_MASK_KEY_PRESS};
	e = xcb_request_check(dpy, xcb_change_window_attributes_checked(dpy,
				screen->root, XCB_CW_EVENT_MASK, values));
	if (e != NULL) {
		xcb_disconnect(dpy);
		DEBUGP("Error code: %d\n", e->error_code);
		DEBUGP("Another window manager is running.\n");
		exit(EXIT_FAILURE);
	}
	free(e);
}

void button_press_event(xcb_generic_event_t *ev)
{
	DEBUG("Button was pressed.");
}

void key_press_event(xcb_generic_event_t *ev)
{
	unsigned int i = 0;
	xcb_key_press_event_t *ke = (xcb_key_press_event_t *)ev;
	DEBUGP("[+] Keypress code:%d mod:%d\n", ke->detail, ke->state);
	xcb_keysym_t keysym = keycode_to_keysym(ke->detail);

	switch (cur_state) {
	case OPERATOR_STATE:
		for (i = 0; i < LENGTH(operators); i++) {
			if (keysym == operators[i].sym && EQUALMODS(operators[i].mod, ke->state)) {
				operator_func = operators[i].func;
				cur_state = COUNT_STATE;
				break;
			}
		}
		break;
	case COUNT_STATE:
		if (EQUALMODS(count_mod, ke->state) && XK_1 <= keysym &&
				keysym <= XK_9) {
			/* Get a value between 0 and 1.  */
			cur_count = keysym - XK_0;
			cur_state = MOTION_STATE;
			break;
		}
	case MOTION_STATE:
		for (i = 0; i < LENGTH(motions); i++) {
			if (keysym == motions[i].sym && EQUALMODS(motions[i].mod, ke->state)) {
				operator_func(motions[i].type, cur_count);
				cur_state = OPERATOR_STATE;
				/* Reset so that kc is equivalent to k1c. */
				cur_count = 1;
			}
		}
	}
	for (i = 0; i < LENGTH(keys); i++)
		if (keysym == keys[i].sym && EQUALMODS(keys[i].mod, ke->state)
				&& keys[i].func && keys[i].mode == cur_mode)
			keys[i].func(&keys[i].arg);
}

void spawn(const Arg *arg)
{
	if (fork())
		return;
	if (dpy)
		close(screen->root);
	setsid();
	execvp((char *)arg->cmd[0], (char **)arg->cmd);
	DEBUG("SPAWN");
	fprintf(stderr, "howm: execvp %s\n", (char *)arg->cmd[0]);
	perror(" failed");
	exit(EXIT_SUCCESS);
}

void map_request_event(xcb_generic_event_t *ev)
{
	xcb_window_t transient = 0;
	xcb_get_window_attributes_reply_t *wa;
	xcb_map_request_event_t *me = (xcb_map_request_event_t *)ev;

	wa = xcb_get_window_attributes_reply(dpy, xcb_get_window_attributes(dpy, me->window), NULL);
	if (!wa || wa->override_redirect || win_to_client(me->window)) {
		free(wa);
		return;
	}
	DEBUG("Mapping request");
	/* Rule stuff needs to be here. */
	Client *c = client_from_window(me->window);

	/* Assume that transient windows MUST float. */
	xcb_icccm_get_wm_transient_for_reply(dpy, xcb_icccm_get_wm_transient_for(dpy, me->window), &transient, NULL);
	c->is_floating = c->is_transient = transient ? true : false;
	arrange_windows();
	xcb_map_window(dpy, c->win);
	update_focused_client(c);
}

Client *client_from_window(xcb_window_t w)
{
	Client *c = (Client *)calloc(1, sizeof(Client));
	Client *t = prev_client(head); /* Get the last element. */

	if (!c)
		err(EXIT_FAILURE, "Can't allocate memory for client.");
	if (!head)
		head = c;
	else if (t)
		t->next = c;
	else
		head->next = c;
	c->win = w;
	unsigned int vals[1] = {XCB_EVENT_MASK_PROPERTY_CHANGE |
		(FOCUS_MOUSE ? XCB_EVENT_MASK_ENTER_WINDOW : 0)};
	xcb_change_window_attributes(dpy, c->win, XCB_CW_EVENT_MASK, vals);
	return c;
}

void save_workspace(int i)
{
	if (i < 0 || i >= WORKSPACES)
		return;
	workspaces[i].layout = cur_layout;
	workspaces[i].current = current;
	workspaces[i].head = head;
	workspaces[i].prev_foc = prev_foc;
}

void select_workspace(int i)
{
	save_workspace(cur_workspace);
	cur_layout = workspaces[i].layout;
	current = workspaces[i].current;
	head = workspaces[i].head;
	prev_foc = workspaces[i].prev_foc;
	cur_workspace = i;
}

Client *win_to_client(xcb_window_t win)
{
	bool found;
	int w = 0, cw = cur_workspace;
	Client *c = NULL;
	for (found = false; w < WORKSPACES && !found; ++w)
		for (select_workspace(w), c = head; c && !(found = (win == c->win)); c = c->next)
			;
	if (cw != w-1)
		select_workspace(cw);
	return c;
}

xcb_keysym_t keycode_to_keysym(xcb_keycode_t code)
{
	xcb_keysym_t sym;
	xcb_key_symbols_t *syms = xcb_key_symbols_alloc(dpy);
	if (!syms)
		return 0;
	sym = xcb_key_symbols_get_keysym(syms, code, 0);
	xcb_key_symbols_free(syms);
	return sym;
}

xcb_keycode_t *keysym_to_keycode(xcb_keysym_t sym)
{
	xcb_keycode_t *code;
	xcb_key_symbols_t *syms = xcb_key_symbols_alloc(dpy);
	if (!syms)
		return NULL;
	code = xcb_key_symbols_get_keycode(syms, sym);
	xcb_key_symbols_free(syms);
	return code;
}

Client *prev_client(Client *c)
{
	if (!c || !head->next)
		return NULL;
	Client *p;
	for (p = head; p->next && p->next != c; p = p->next)
		;
	return p;
}

Client *next_client(Client *c)
{
	if (!c || !head->next)
		return NULL;
	if (c->next)
		return c->next;
	return head;
}

void remove_from_workspace(Client *c)
{
	if (head == c)
		head = NULL;
	Client *p = prev_client(c);
	p->next = c->next;
}

void arrange_windows(void)
{
	if (!head)
		return;
	DEBUG("Arranging");
	layout_handler[head->next ? cur_layout : ZOOM]();
	howm_info();
}

void grid(void)
{
	DEBUG("GRID");
}

void zoom(void)
{
	DEBUG("ZOOM");
	Client *c;
	for (c = head; c; c = c->next)
		if (!FFT(c))
			move_resize(dpy, c->win, ZOOM_GAP ? true : false,
					0, 0, screen_width, screen_height);
}

void move_resize(xcb_connection_t *con, xcb_window_t win, bool draw_gap,
		int x, int y, int w, int h)
{
	unsigned int position[] = {x, y, w, h};
	if (draw_gap) {
		position[0] += GAP;
		position[1] += GAP;
		position[2] -= 2 * GAP;
		position[3] -= 2 * GAP;
	}

	xcb_configure_window(con, win, MOVE_RESIZE_MASK, position);
}

void update_focused_client(Client *c)
{
	if (!head) {
		prev_foc = current = NULL;
		xcb_delete_property(dpy, screen->root, net_atoms[NET_ACTIVE_WINDOW]);
		return;
	} else if (c == prev_foc) {
		current = (prev_foc ? prev_foc : head);
		prev_foc = prev_client(current);
	} else if (c != current) {
		prev_foc = current;
		current = c;
	}

	DEBUG("UPDATING");
	unsigned int all = 0, fullscreen = 0, float_trans = 0;
	for (c = head; c; c = c->next, ++all) {
		if (FFT(c))
			fullscreen++;
			if (!c->is_fullscreen)
				float_trans++;
	}
	xcb_window_t windows[all];
	windows[(current->is_floating || current->is_transient) ? 0 : fullscreen] = current->win;
	c = head;
	for (fullscreen += FFT(current) ? 1 : 0; c; c = c->next) {
		set_border_width(dpy, c->win, (c->is_fullscreen ||
					!head->next) ? 0 : BORDER_PX);
		xcb_change_window_attributes(dpy, c->win, XCB_CW_BORDER_PIXEL,
			(c == current ? &border_focus : &border_unfocus));
		if (c != current)
			windows[c->is_fullscreen ? --fullscreen : FFT(c) ?
				--float_trans : --all] = c->win;
	}

	for (float_trans = 0; float_trans <= all; ++float_trans)
		elevate_window(windows[all - float_trans]);

	xcb_change_property(dpy, XCB_PROP_MODE_REPLACE, screen->root,
			net_atoms[NET_ACTIVE_WINDOW], XCB_ATOM_WINDOW, 32, 1, &current->win);
	xcb_set_input_focus(dpy, XCB_INPUT_FOCUS_POINTER_ROOT, current->win,
			XCB_CURRENT_TIME);
	arrange_windows();
}

void grab_keys(void)
{
	/* TODO: optimise this so that it doesn't call xcb_grab_key for all
	 * keys, as some are repeated due to modes. Perhaps XCB does this
	 * already? */
	DEBUG("Grabbing keys.");
	xcb_keycode_t *keycode;
	unsigned int i;
	xcb_ungrab_key(dpy, XCB_GRAB_ANY, screen->root, XCB_MOD_MASK_ANY);
	for (i = 0; i < LENGTH(keys); i++) {
		keycode = keysym_to_keycode(keys[i].sym);
		grab_keycode(keycode, keys[i].mod);
	}

	for (i = 0; i < LENGTH(operators); i++) {
		keycode = keysym_to_keycode(operators[i].sym);
		grab_keycode(keycode, operators[i].mod);
	}

	for (i = 0; i < LENGTH(motions); i++) {
		keycode = keysym_to_keycode(motions[i].sym);
		grab_keycode(keycode, motions[i].mod);
	}
}

void grab_keycode(xcb_keycode_t *keycode, const int mod)
{
	unsigned int j, k;
	unsigned int mods[] = {0, XCB_MOD_MASK_LOCK};
	for (j = 0; keycode[j] != XCB_NO_SYMBOL; j++)
		for (k = 0; k < LENGTH(mods); k++)
			xcb_grab_key(dpy, 1, screen->root, mod |
				mods[k], keycode[j], XCB_GRAB_MODE_ASYNC,
				XCB_GRAB_MODE_ASYNC);

}

void set_border_width(xcb_connection_t *con, xcb_window_t win, int w)
{
	unsigned int width[1] = {w};
	xcb_configure_window(con, win, XCB_CONFIG_WINDOW_BORDER_WIDTH, width);
}

void elevate_window(xcb_window_t win)
{
	unsigned int stack_mode[1] = {XCB_STACK_MODE_ABOVE};
	xcb_configure_window(dpy, win, XCB_CONFIG_WINDOW_STACK_MODE, stack_mode);
}

void get_atoms(char **names, xcb_atom_t *atoms, unsigned int cnt)
{
	xcb_intern_atom_reply_t *reply;
	xcb_intern_atom_cookie_t cookies[cnt];
	unsigned int i;

	for (i = 0; i < cnt; i++)
		cookies[i] = xcb_intern_atom(dpy, 0, strlen(names[i]), names[i]);
	for (i = 0; i < cnt; i++) {
		reply = xcb_intern_atom_reply(dpy, cookies[i], NULL);
		if (reply) {
			atoms[i] = reply->atom;
			free(reply);
		} else {
			DEBUGP("WARNING: the atom %s has not been registered by howm.\n", names[i]);
		}
	}
}

void stack(void)
{
	Client *c = NULL;
	bool vert = (cur_layout == VSTACK);
	int span = vert ? screen_height :  screen_width;
	int client_span, i, n, client_y = 0, client_x = 0;

	n = get_non_tff_count();
	/* TODO: Need to take into account when this has remainders. */
	/* TODO: Fix doubling of gap between windows. */
	client_span = (span / n) - (2 * BORDER_PX);
	DEBUG("STACK")
	DEBUGP("span: %d\n", span);
	DEBUGP("client_span: %d\n", client_span);

	if (vert) {
		move_resize(dpy, head->win, BORDER_PX, client_y,
			true, screen_width - (2 * BORDER_PX), client_span);
		client_y += (BORDER_PX + (span / n));
	} else {
		move_resize(dpy, head->win, client_x, BORDER_PX,
			true, client_span, screen_height - (2 * BORDER_PX));
		client_x += (BORDER_PX + (span / n));
	}

	if (!head->next)
		return;

	for (c = head->next, i = 0; i < n - 1; c = c->next, i++) {
		if (vert) {
			DEBUGP("client_y: %d\n", client_y);
			move_resize(dpy, c->win, false, GAP, client_y,
					screen_width - (2 * (BORDER_PX + GAP)),
					client_span - GAP - BORDER_PX);
			client_y += (BORDER_PX + client_span);
		} else {
			move_resize(dpy, c->win, false, client_x, GAP,
					client_span - GAP - BORDER_PX,
					screen_height - (2 * (BORDER_PX + GAP)));
			client_x += (BORDER_PX + client_span);
		}
	}
}

int get_non_tff_count(void)
{
	int n = 0;
	Client *c = NULL;

	for (c = head; c; c = c->next)
		if (!FFT(c))
			n++;
		else
			break;
	return n;

}

void destroy_event(xcb_generic_event_t *ev)
{
	DEBUG("DESTROY");
	xcb_destroy_notify_event_t *de = (xcb_destroy_notify_event_t *) ev;
	Client *c = win_to_client(de->window);
	if (c)
		remove_client(c);
}

void remove_client(Client *c)
{
	Client **temp = NULL;
	int w = 0, cw = cur_workspace;
	bool found;
	for (found = false; w < WORKSPACES && !found; w++)
		for (temp = &head, select_workspace(cw); temp &&
				!(found = *temp == c); temp = &(*temp)->next)
			;
	*temp = c->next;
	if (c == prev_foc)
		prev_foc = prev_client(c);
	if (c == head)
		head = NULL;
	if (c == current || !head->next)
		update_focused_client(prev_foc);
	free(c);
	c = NULL;
	if (cw == w - 1)
		arrange_windows();
	else
		select_workspace(cw);
}

void howm_info(void)
{
	int cw = cur_workspace;
	int w, n;
	Client *c;
	for (w = 0; w < WORKSPACES; w++) {
		for (select_workspace(w), c = head, n = 0; c; c = c->next, n++)
			;
		printf("m:%d l:%d n:%d w:%d cw:%d s:%d\n", cur_mode, cur_layout, n, w,
			cur_workspace == cw, cur_state);
	}
	if (cw != w - 1)
		select_workspace(cw);
}

void enter_event(xcb_generic_event_t *ev)
{
	xcb_enter_notify_event_t *ee = (xcb_enter_notify_event_t *) ev;
	Client *c = NULL;
	if (!FOCUS_MOUSE)
		return;
	DEBUG("enter_event");
	c = win_to_client(ee->event);
	if (c)
		update_focused_client(c);
}

void fibonacci(void)
{
	Client *c = NULL;
	int ch = screen_height - (BORDER_PX + GAP);
	int cw = screen_width - (BORDER_PX + GAP);
	for (c = head; c; c = c->next) {
		if FFT(c)
			continue;
		/* TODO: Actual fibonacci stuff. */
	}
}

void move_down(Client *c)
{
	Client *prev = prev_client(c);
	Client *n = (c->next) ? c->next : head;
	if (!prev)
		return;
	if (head == c)
		head = n;
	else
		prev->next = c->next;
	c->next = (c->next) ? n->next : n;
	if (n->next == c->next)
		n->next = c;
	else
		head = c;
	arrange_windows();
}

void move_up(Client *c)
{
	Client *p = prev_client(c);
	Client *pp = NULL;
	if (!p)
		return;
	if (p->next)
		for (pp = head; pp && pp->next != p; pp = pp->next)
			;
	if (pp)
		pp->next = c;
	else
		head = (head == c) ? c->next : c;
	p->next = (c->next == head) ? c : c->next;
	c->next = (c->next == head) ? NULL : p;
	arrange_windows();
}

void focus_next(void)
{
	if (!current || !head->next)
		return;
	DEBUG("focus_next");
	update_focused_client(current->next ? current->next : head);
}

void focus_prev(void)
{
	if (!current || !head->next)
		return;
	DEBUG("focus_prev");
	prev_foc = current;
	update_focused_client(prev_client(prev_foc));
}

void change_workspace(const Arg *arg)
{
	if (arg->i >= WORKSPACES || arg->i < 0 || arg->i == cur_workspace)
		return;
	prev_workspace = cur_workspace;
	select_workspace(arg->i);
	for (Client *c = head; c; c = c->next)
		xcb_map_window(dpy, c->win);
	select_workspace(prev_workspace);
	for (Client *c = head; c; c = c->next)
		xcb_unmap_window(dpy, c->win);
	select_workspace(arg->i);
	arrange_windows();
	update_focused_client(current);
	howm_info();
}

void previous_workspace(void)
{
	const Arg arg = {.i = cur_workspace < 1 ? WORKSPACES - 1 :
				cur_workspace - 1};
	change_workspace(&arg);
}

void last_workspace(void)
{
	const Arg arg = {.i = prev_workspace};
	change_workspace(&arg);
}

void next_workspace(void)
{
	const Arg arg = {.i = (cur_workspace + 1) % WORKSPACES};
	change_workspace(&arg);
}

void change_layout(const Arg *arg)
{
	if (arg->i == cur_layout || arg->i >= END_LAYOUT || arg->i < 0)
		return;
	prev_layout = cur_layout;
	cur_layout = arg->i;
	arrange_windows();
	update_focused_client(current);
	DEBUGP("Changed layout to %d\n", cur_layout);
}

void previous_layout(void)
{
	const Arg arg = {.i =  cur_layout < 1 ? END_LAYOUT - 1 : cur_layout - 1};
	change_layout(&arg);
}

void next_layout(void)
{
	const Arg arg = {.i = (cur_layout + 1) % END_LAYOUT};
	change_layout(&arg);
}

void last_layout(void)
{
	const Arg arg = {.i = prev_layout};
	change_layout(&arg);
}

void change_mode(const Arg *arg)
{
	if (arg->i >= END_MODES || arg->i == cur_mode)
		return;
	cur_mode = arg->i;
	DEBUGP("Changed mode to %d\n", cur_mode);
}

void op_kill(const int type, int count)
{
	if (type == WORKSPACE) {
		DEBUGP("Killing %d workspaces.\n", count);
		while (count > 0) {
			/* Count being here is intentional as workspaces are
			 * zero indexed. */
			count--;
			kill_workspace((cur_workspace + count) % WORKSPACES);
		}

	} else if (type == CLIENT) {
		DEBUGP("Killing %d clients.\n", count);
		while (count > 0) {
			kill_client();
			count--;
		}
	}
}

void kill_client(void)
{
	if (!current)
		return;
	/* TODO: Kill the window in a nicer way and get it to consistently die. */
	xcb_kill_client(dpy, current->win);
	DEBUG("Killing Client");
	remove_client(current);
}

void kill_workspace(const int ws)
{
	Arg arg = {.i = ws};
	change_workspace(&arg);
	while (head != NULL)
		kill_client();
}

void op_move_down(const int type, int count)
{
	if (type == WORKSPACE) {
		/* TODO: Make this so that an entire desktop(s) can be moved. */
		return;
	} else if (type == CLIENT) {
		Client *c = current;
		while (count > 0) {
			move_down(c);
			c = next_client(c);
			count--;
		}
	}
}

void op_move_up(const int type, int count)
{
	if (type == WORKSPACE) {
		/* TODO: Make this so that an entire desktop(s) can be moved. */
		return;
	} else if (type == CLIENT) {
		Client *c = current;
		while (count > 0) {
			move_up(c);
			c = prev_client(c);
			count--;
		}
	}
}

void move_current_down(void)
{
	move_down(current);
}

void move_current_up(void)
{
	move_up(current);
}

void client_to_workspace(Client *c, const int ws)
{
	if (!c || ws == cur_workspace)
		return;
	Client *last;
	Client *prev = prev_client(c);
	int cw = ws;
	prev->next = next_client(c);
	change_workspace(ws);
	last = prev_client(head);
}

