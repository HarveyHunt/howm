#include <err.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <X11/keysym.h>
#include <X11/X.h>
#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_keysyms.h>

/**
 * @file howm.c
 */

/**
 * @brief howm
 */

/** Calculates a mask that can be applied to a window in order to reconfigure a
 * window. */
#define MOVE_RESIZE_MASK (XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | \
			  XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT)
/** Ensures that the number lock doesn't intefere with checking the equality
 * of two modifier masks.*/
#define CLEANMASK(mask) (mask & ~(numlockmask | XCB_MOD_MASK_LOCK))
/** Wraps up the comparison of modifier masks into a neat package. */
#define EQUALMODS(mask, omask) (CLEANMASK(mask) == CLEANMASK(omask))
/** Calculates the length of an array. */
#define LENGTH(x) (unsigned int)(sizeof(x) / sizeof(*x))
/** Checks to see if a client is floating, fullscreen or transient. */
#define FFT(client) (c->is_transient || c->is_floating || c->is_fullscreen)

/**
 * @brief Represents an argument.
 *
 * Used to hold data that is sent as a parameter to a function when called as a
 * result of a keypress.
 */
typedef union {
	const char	**cmd;  /**< Represents a command that will be called by a shell.  */
	float		f;      /**< Commonly used for scaling operations. */
	int		i;      /**< Usually used for specifying workspaces or clients. */
} Arg;

/**
 * @brief Represents a key.
 *
 * Holds information relative to a key, such as keysym and the mode during
 * which the keypress can be seen as valid.
 */
typedef struct {
	int		mod;            /**< The mask of the modifiers pressed. */
	unsigned int	mode;           /**< The mode within which this keypress is valid. */
	xcb_keysym_t	sym;            /**< The keysym of the pressed key. */
	void (*func)(const Arg *);      /**< The function to be called when this key is pressed. */
	const Arg	arg;            /**< The argument passed to the above function. */
} Key;

/**
 * @brief Represents an operator.
 *
 * Operators perform an action upon one or more targets (identified by
 * motions).
 */
typedef struct {
	int		mod;                                    /**< The mask of the modifiers pressed. */
	xcb_keysym_t	sym;                                    /**< The keysym of the pressed key. */
	unsigned int	mode;                                   /**< The mode within which this keypress is valid. */
	void (*func)(const int unsigned type, const int cnt);   /**< The function to be
								 * called when the key is pressed. */
} Operator;

/**
 * @brief Represents a motion.
 *
 * A motion can be used to target an operation at something specific- such as a
 * client or workspace.
 *
 * For example:
 *
 * q4c (Kill, 4, Clients).
 */
typedef struct {
	int		mod;    /**< The mask of the modifiers pressed. */
	xcb_keysym_t	sym;    /**< The keysym of the pressed key. */
	unsigned int	type;   /**< Represents whether the motion is for clients, WS etc. */
} Motion;

/**
 * @brief Represents a button.
 *
 * Allows the mapping of a button to a function, as is done with the Key struct
 * for keys.
 */
typedef struct {
	int		mod;            /**< The mask of the modifiers pressed.  */
	short int	button;         /**< The button that was pressed. */
	void (*func)(const Arg *);      /**< The function to be called when the
					* button is pressed. */
	const Arg	arg;            /**< The argument passed to the above function. */
} Button;

/**
 * @brief Represents a client that is being handled by howm.
 *
 * All the attributes that are needed by howm for a client are stored here.
 */
typedef struct Client {
	struct Client *next;           /**< Clients are stored in a linked list-
					* this represents the client after this one. */
	bool		is_fullscreen;  /**< Is the client fullscreen? */
	bool		is_floating;    /**< Is the client floating? */
	bool		is_transient;   /**< Is the client transient?
					* Defined at: http://standards.freedesktop.org/wm-spec/wm-spec-latest.html*/
	xcb_window_t	win;            /**< The window that this client represents. */
	uint16_t x; /**< The x coordinate of the client. */
	uint16_t y; /**< The y coordinate of the client. */
	uint16_t w; /**< The width of the client.*/
	uint16_t h; /**< The height of the client.*/
} Client;

/**
 * @brief Represents a workspace, which stores clients.
 *
 * Clients are stored as a linked list. Changing to a different workspace will
 * cause different clients to be rendered on the screen.
 */
typedef struct {
	int	layout;         /**< The current layout of the WS, as defined in the
				* layout enum. */
	Client *head;           /**< The start of the linked list. */
	Client *prev_foc;       /**< The last focused client. This is seperate to
				* the linked list structure. */
	Client *current;        /**< The client that is currently in focus. */
} Workspace;

/* Operators */
static void op_kill(const int type, int cnt);
static void op_move_up(const int type, int cnt);
static void op_move_down(const int type, int cnt);
static void op_focus_down(const int type, int cnt);
static void op_focus_up(const int type, int cnt);

/* Clients */
static void move_current_down(const Arg *arg);
static void move_current_up(const Arg *arg);
static void kill_client(void);
static void move_down(Client *c);
static void move_up(Client *c);
static Client *next_client(Client *c);
static void focus_next_client(const Arg *arg);
static void focus_prev_client(const Arg *arg);
static void update_focused_client(Client *c);
static Client *prev_client(Client *c);
static Client *create_client(xcb_window_t w);
static void remove_client(Client *c);
static Client *find_client_by_win(xcb_window_t w);
static void client_to_ws(Client *c, const int ws);
static void current_to_ws(const Arg *arg);
static void draw_clients(void);
static void change_client_geom(Client *c, uint16_t x, uint16_t y, uint16_t w, uint16_t h);

/* Workspaces */
static void kill_ws(const int ws);
static void focus_next_ws(const Arg *arg);
static void focus_prev_ws(const Arg *arg);
static void focus_last_ws(const Arg *arg);
static void change_ws(const Arg *arg);
static void save_ws(int i);
static void select_ws(int i);
static int prev_ws(int ws);
static int next_ws(int ws);
static int correct_ws(int ws);
static void move_ws_down(int ws);
static void move_ws_up(int ws);
static void move_ws(int s_ws, int d_ws);

/* Layouts */
static void change_layout(const Arg *arg);
static void next_layout(const Arg *arg);
static void previous_layout(const Arg *arg);
static void last_layout(const Arg *arg);
static void stack(void);
static void grid(void);
static void zoom(void);
static void arrange_windows(void);

/* Modes */
static void change_mode(const Arg *arg);

/* Events */
static void enter_event(xcb_generic_event_t *ev);
static void destroy_event(xcb_generic_event_t *ev);
static void button_press_event(xcb_generic_event_t *ev);
static void key_press_event(xcb_generic_event_t *ev);
static void map_event(xcb_generic_event_t *ev);
static void configure_event(xcb_generic_event_t *ev);
static void unmap_event(xcb_generic_event_t *ev);

/* XCB */
static void grab_keys(void);
static xcb_keycode_t *keysym_to_keycode(xcb_keysym_t sym);
static void grab_keycode(xcb_keycode_t *keycode, const int mod);
static void elevate_window(xcb_window_t win);
static void move_resize(xcb_window_t win, bool draw_gap, int x, int y, int w, int h);
static void set_border_width(xcb_window_t win, int w);
static void get_atoms(char **names, xcb_atom_t *atoms);
static void check_other_wm(void);
static xcb_keysym_t keycode_to_keysym(xcb_keycode_t keycode);

/* Misc */
static void howm_info(void);
static int get_non_tff_count(void);
static uint32_t get_colour(char *colour);
static void spawn(const Arg *arg);
static void setup(void);
static void move_ws_or_client(const int type, int cnt, bool up);
static void focus_window(xcb_window_t win);

enum { ZOOM, GRID, HSTACK, VSTACK, END_LAYOUT };
enum { OPERATOR_STATE, COUNT_STATE, MOTION_STATE, END_STATE };
enum { NORMAL, FOCUS, RESIZE, END_MODES };
enum { COMMAND, OPERATOR, MOTION, END_TYPE };
enum { CLIENT, WORKSPACE };
enum { NET_WM_STATE_FULLSCREEN, NET_SUPPORTED, NET_WM_STATE,
	NET_ACTIVE_WINDOW
	};
enum { WM_DELETE_WINDOW, WM_PROTOCOLS };

/* Handlers */
static void(*handler[XCB_NO_OPERATION]) (xcb_generic_event_t *) = {
	[XCB_BUTTON_PRESS] = button_press_event,
	[XCB_KEY_PRESS] = key_press_event,
	[XCB_MAP_REQUEST] = map_event,
	[XCB_DESTROY_NOTIFY] = destroy_event,
	[XCB_ENTER_NOTIFY] = enter_event,
	[XCB_CONFIGURE_NOTIFY] = configure_event,
	[XCB_UNMAP_NOTIFY] = unmap_event
};

static void(*layout_handler[]) (void) = {
	[GRID] = grid,
	[ZOOM] = zoom,
	[HSTACK] = stack,
	[VSTACK] = stack
};

static void (*operator_func)(const unsigned int type, int cnt);

static xcb_connection_t *dpy;
static char *WM_ATOM_NAMES[] = { "WM_DELETE_WINDOW", "WM_PROTOCOLS" };
static char *NET_ATOM_NAMES[] = { "_NET_WM_STATE_FULLSCREEN", "_NET_SUPPORTED",
				  "_NET_WM_STATE",	      "_NET_ACTIVE_WINDOW"
				};
static xcb_atom_t wm_atoms[LENGTH(WM_ATOM_NAMES)],
	net_atoms[LENGTH(NET_ATOM_NAMES)];
static xcb_screen_t *screen;
static int numlockmask;
static Client *head, *prev_foc, *current;
/* We don't need the range of unsigned, so this prevents a conversion later. */
static int last_ws, cur_layout, prev_layout;
static int cur_ws = 1;
static unsigned int border_focus, border_unfocus;
static unsigned int cur_mode, cur_state = OPERATOR_STATE;
static unsigned int cur_cnt = 1;
static uint16_t screen_height, screen_width;

#include "config.h"

/* Add comments so that splint ignores this as it doesn't support variadic
 * macros.
 */
/*@ignore@*/
#ifdef DEBUG_ENABLE
/** Output debugging information using puts. */
#       define DEBUG(x) puts(x);
/** Output debugging information using printf to allow for formatting. */
#       define DEBUGP(x, ...) printf(x, ## __VA_ARGS__);
#else
#       define DEBUG(x) do {} while (0)
#       define DEBUGP(x, ...) do {} while (0)
#endif
/*@end@*/

/**
 * @brief Occurs when howm first starts.
 *
 * A connection to the X11 server is attempted and keys are then grabbed.
 *
 * Atoms are gathered.
 */
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

	get_atoms(NET_ATOM_NAMES, net_atoms);
	get_atoms(WM_ATOM_NAMES, wm_atoms);

	border_focus = get_colour(BORDER_FOCUS);
	border_unfocus = get_colour(BORDER_UNFOCUS);

	cur_layout = workspaces[cur_ws].layout;
}

/**
 * @brief Converts a hexcode colour into an X11 colourmap pixel.
 *
 * @param colour A string of the format "#FFFFFF", that will be interpreted as
 * a colour code.
 *
 * @return An X11 colourmap pixel.
 */
uint32_t get_colour(char *colour)
{
	uint32_t pixel;
	uint16_t r, g, b;
	xcb_alloc_color_reply_t *rep;
	xcb_colormap_t map = screen->default_colormap;

	unsigned long int rgb = strtol(++colour, NULL, 16);

	r = ((rgb >> 16) & 0xFF) * 257;
	g = ((rgb >> 8) & 0xFF) * 257;
	b = (rgb & 0xFF) * 257;
	rep = xcb_alloc_color_reply(dpy, xcb_alloc_color(dpy, map,
				    r, g, b), NULL);
	if (!rep)
		err(EXIT_FAILURE, "ERROR: Can't allocate the colour %s\n",
		    colour);
	pixel = rep->pixel;
	free(rep);
	return pixel;
}

/**
 * @brief The code that glues howm together...
 */
int main(int argc, char *argv[])
{
	xcb_generic_event_t *ev;

	dpy = xcb_connect(NULL, NULL);
	if (xcb_connection_has_error(dpy))
		err(EXIT_FAILURE, "Can't open Xconnection\n");
	setup();
	check_other_wm();
	while (!xcb_connection_has_error(dpy)) {
		if (!xcb_flush(dpy))
			err(EXIT_FAILURE, "Failed to flush X connection\n");
		ev = xcb_wait_for_event(dpy);
		if (handler[ev->response_type])
			handler[ev->response_type](ev);
	}
	return 1;
}

/**
 * @brief Try to detect if another WM exists.
 *
 * If another WM exists (this can be seen by whether it has registered itself
 * with the X11 server) then howm will exit.
 */
void check_other_wm(void)
{
	xcb_generic_error_t *e;
	uint32_t values[1] = { XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
			       XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
			       XCB_EVENT_MASK_BUTTON_PRESS |
			       XCB_EVENT_MASK_KEY_PRESS
			     };

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

/**
 * @brief Process a button press.
 *
 * @param ev The button press event.
 */
void button_press_event(xcb_generic_event_t *ev)
{
	/* FIXME: be->event doesn't seem to match with any windows managed by howm.*/
	xcb_button_press_event_t *be = (xcb_button_press_event_t *)ev;

	DEBUG("button_press_event");
	if (FOCUS_MOUSE_CLICK && be->detail == XCB_BUTTON_INDEX_1)
		focus_window(be->event);
}

/**
 * @brief Process a key press.
 *
 * This function implements an FSA that determines which command to run, as
 * well as with what targets and how many times.
 *
 * An keyboard input of the form qc (Assuming the correct mod keys have been
 * pressed) will lead to one client being killed- howm assumes no count means
 * perform the operation once. This is the behaviour that vim uses.
 *
 * Only counts as high as 9 are acceptable- I feel that any higher would just
 * be pointless.
 *
 * @param ev A keypress event.
 */
void key_press_event(xcb_generic_event_t *ev)
{
	unsigned int i = 0;
	xcb_keysym_t keysym;
	xcb_key_press_event_t *ke = (xcb_key_press_event_t *)ev;

	DEBUGP("[+] Keypress code:%d mod:%d\n", ke->detail, ke->state);
	keysym = keycode_to_keysym(ke->detail);
	switch (cur_state) {
	case OPERATOR_STATE:
		for (i = 0; i < LENGTH(operators); i++) {
			if (keysym == operators[i].sym && EQUALMODS(operators[i].mod, ke->state)
			    && operators[i].mode == cur_mode) {
				operator_func = operators[i].func;
				cur_state = COUNT_STATE;
				break;
			}
		}
		break;
	case COUNT_STATE:
		if (EQUALMODS(COUNT_MOD, ke->state) && XK_1 <= keysym &&
		    keysym <= XK_9) {
			/* Get a value between 1 and 9 inclusive.  */
			cur_cnt = keysym - XK_0;
			cur_state = MOTION_STATE;
			break;
		}
	case MOTION_STATE:
		for (i = 0; i < LENGTH(motions); i++) {
			if (keysym == motions[i].sym && EQUALMODS(motions[i].mod, ke->state)) {
				operator_func(motions[i].type, cur_cnt);
				cur_state = OPERATOR_STATE;
				/* Reset so that qc is equivalent to q1c. */
				cur_cnt = 1;
			}
		}
	}
	for (i = 0; i < LENGTH(keys); i++)
		if (keysym == keys[i].sym && EQUALMODS(keys[i].mod, ke->state)
		    && keys[i].func && keys[i].mode == cur_mode)
			keys[i].func(&keys[i].arg);
}

/**
 * @brief Spawns a command.
 */
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

/**
 * @brief Handles mapping requests.
 *
 * When an X window wishes to be displayed, it send a mapping request. This
 * function processes that mapping request and inserts the new client (created
 * from the map requesting window) into the list of clients for the current
 * workspace.
 *
 * @param ev A mapping request event.
 */
void map_event(xcb_generic_event_t *ev)
{
	xcb_window_t transient = 0;
	xcb_get_window_attributes_reply_t *wa;
	xcb_map_request_event_t *me = (xcb_map_request_event_t *)ev;
	Client *c;

	wa = xcb_get_window_attributes_reply(dpy, xcb_get_window_attributes(dpy, me->window), NULL);
	if (!wa || wa->override_redirect || find_client_by_win(me->window)) {
		free(wa);
		return;
	}
	free(wa);
	DEBUG("Mapping request");
	/* Rule stuff needs to be here. */
	c = create_client(me->window);

	/* Assume that transient windows MUST float. */
	xcb_icccm_get_wm_transient_for_reply(dpy, xcb_icccm_get_wm_transient_for(dpy, me->window), &transient, NULL);
	c->is_floating = c->is_transient = transient ? true : false;
	arrange_windows();
	xcb_map_window(dpy, c->win);
	update_focused_client(c);
}

/**
 * @brief Convert a window into a client.
 *
 * @param w A valid xcb window.
 *
 * @return A client that has already been inserted into the linked list of
 * clients.
 */
Client *create_client(xcb_window_t w)
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
	unsigned int vals[1] = { XCB_EVENT_MASK_PROPERTY_CHANGE |
				 (FOCUS_MOUSE ? XCB_EVENT_MASK_ENTER_WINDOW : 0)
			       };
	xcb_change_window_attributes(dpy, c->win, XCB_CW_EVENT_MASK, vals);
	return c;
}

/**
 * @brief Saves the information about a current workspace.
 *
 * @param i The index of the workspace to be saved. Note: Workspaces begin at
 * index 1.
 */
void save_ws(int i)
{
	if (i < 1 || i > WORKSPACES)
		return;
	workspaces[i].layout = cur_layout;
	workspaces[i].current = current;
	workspaces[i].head = head;
	workspaces[i].prev_foc = prev_foc;
}

/**
 * @brief Reloads the information about a workspace and sets it as the current
 * workspace.
 *
 * @param i The index of the workspace to be reloaded and set as current. Note:
 * Workspaces begin at index 1.
 */
void select_ws(int i)
{
	save_ws(cur_ws);
	cur_layout = workspaces[i].layout;
	current = workspaces[i].current;
	head = workspaces[i].head;
	prev_foc = workspaces[i].prev_foc;
	cur_ws = i;
}

/**
 * @brief Search workspaces for a window, returning the client that it belongs
 * to.
 *
 * During searching, the current workspace is changed so that all workspaces
 * can be searched. Upon finding the client, the original workspace is
 * restored.
 *
 * @param win A valid XCB window that is used when searching all clients across
 * all desktops.
 *
 * @return The found client.
 */
Client *find_client_by_win(xcb_window_t win)
{
	bool found;
	int w = 1, cw = cur_ws;
	Client *c = NULL;

	for (found = false; w < WORKSPACES && !found; ++w)
		for (select_ws(w), c = head; c && !(found = (win == c->win)); c = c->next)
			;
	if (cw != w)
		select_ws(cw);
	return c;
}

/**
 * @brief Convert a keycode to a keysym.
 *
 * @param code An XCB keycode.
 *
 * @return The keysym corresponding to the given keycode.
 */
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

/**
 * @brief Convert a keysym to a keycode.
 *
 * @param sym An XCB keysym.
 *
 * @return The keycode corresponding to the given keysym.
 */
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

/**
 * @brief Find the client before the given client.
 *
 * @param c The client which needs to have its previous found.
 *
 * @return The previous client, so long as the given client isn't NULL and
 * there is more than one client. Else, NULL.
 */
Client *prev_client(Client *c)
{
	if (!c || !head->next)
		return NULL;
	Client *p;
	for (p = head; p->next && p->next != c; p = p->next)
		;
	return p;
}

/**
 * @brief Find the next client.
 *
 * Note: This function wraps around the end of the list of clients. If c is the
 * last item in the list of clients, then the head of the list is returned.
 *
 * @param c The client which needs to have its next found.
 *
 * @return The next client, if c is the last client in the list then this will
 * be head. If c is NULL or there is only one client in the client list, NULL
 * will be returned.
 */
Client *next_client(Client *c)
{
	if (!c || !head->next)
		return NULL;
	if (c->next)
		return c->next;
	return head;
}

/**
 * @brief Call the appropriate layout handler for each layout.
 */
void arrange_windows(void)
{
	if (!head)
		return;
	DEBUG("Arranging");
	layout_handler[head->next ? cur_layout : ZOOM]();
	howm_info();
}

/**
 * @brief Arrange the windows into a grid layout.
 */
void grid(void)
{
	DEBUG("GRID");
	int n = get_non_tff_count();
	if (n == 1) {
		zoom();
		return;
	}

	Client *c = NULL;
	int cols, rows, col_w, i = -1, col_cnt = 0, row_cnt= 0;
	int client_y = BAR_BOTTOM ? 0 : BAR_HEIGHT;
	int col_h = screen_height - BAR_HEIGHT;

	for (cols = 0; cols <= n / 2; cols++)
		if (cols * cols >= n)
			break;
	rows = n / cols;
	col_w = screen_width / cols;
	for (c = head; c; c = c->next) {
		if (FFT(c))
			continue;
		else
			i++;

		if (cols - (n % cols) < (i / rows) + 1)
			rows = n / cols + 1;
		change_client_geom(c, col_cnt * col_w, client_y + (row_cnt * col_h / rows),
				col_w - BORDER_PX, (col_h / rows) - BORDER_PX);
		if (++row_cnt >= rows) {
			row_cnt = 0;
			col_cnt++;
		}
	}
	draw_clients();
}

/**
 * @brief Have one window at a time taking up the entire screen.
 *
 * Sets the geometry of each window in order for the windows to be rendered to
 * take up the entire screen.
 */
void zoom(void)
{
	DEBUG("ZOOM");
	Client *c;
	for (c = head; c; c = c->next)
		if (!FFT(c))
			change_client_geom(c, 0, BAR_BOTTOM ? 0 : BAR_HEIGHT,
					screen_width, screen_height - BAR_HEIGHT);
	draw_clients();
}

/**
 * @brief Change the dimensions and location of a window (win).
 *
 * @param win The window upon which the operations should be performed.
 * @param draw_gap Whether or not to draw useless gaps around the window.
 * @param x The new x location of the top left corner.
 * @param y The new y location of the top left corner.
 * @param w The new width of the window.
 * @param h The new height of the window.
 */
void move_resize(xcb_window_t win, bool draw_gap,
		 int x, int y, int w, int h)
{
	unsigned int position[] = { x, y, w, h };

	if (draw_gap) {
		position[0] += GAP;
		position[1] += GAP;
		position[2] -= 2 * GAP;
		position[3] -= 2 * GAP;
	}
	xcb_configure_window(dpy, win, MOVE_RESIZE_MASK, position);
}

/**
 * @brief Sets c to the active window and gives it input focus. Sorts out
 * border colours as well.
 *
 * @param c The client that is currently in focus.
 */
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
		set_border_width(c->win, (c->is_fullscreen ||
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

/**
 * @brief Let the X11 server know which keys howm is interested in so that howm
 * can be alerted when any of them are pressed.
 *
 * All keys are ungrabbed and then each key in keys, operators and motions are
 * grabbed.
 */
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

	for (i = 0; i < 8; i++) {
		keycode = keysym_to_keycode(XK_1 + i);
		grab_keycode(keycode, COUNT_MOD);
	}
}

/**
 * @brief Grab a keycode, therefore telling the X11 server howm wants to
 * receive events when the key is pressed.
 *
 * @param keycode The keycode to be grabbed.
 * @param mod The modifier that should be pressed down in order for an event
 * for the keypress to be sent to howm.
 */
void grab_keycode(xcb_keycode_t *keycode, const int mod)
{
	unsigned int j, k;
	unsigned int mods[] = { 0, XCB_MOD_MASK_LOCK };

	for (j = 0; keycode[j] != XCB_NO_SYMBOL; j++)
		for (k = 0; k < LENGTH(mods); k++)
			xcb_grab_key(dpy, 1, screen->root, mod |
				     mods[k], keycode[j], XCB_GRAB_MODE_ASYNC,
				     XCB_GRAB_MODE_ASYNC);
}

/**
 * @brief Sets the width of the borders around a window (win).
 *
 * @param win The window that will have its border width changed.
 * @param w The new width of the window's border.
 */
void set_border_width(xcb_window_t win, int w)
{
	unsigned int width[1] = { w };

	xcb_configure_window(dpy, win, XCB_CONFIG_WINDOW_BORDER_WIDTH, width);
}

/**
 * @brief Move a window to the front of all the other windows.
 *
 * @param win The window to be moved.
 */
void elevate_window(xcb_window_t win)
{
	unsigned int stack_mode[1] = { XCB_STACK_MODE_ABOVE };

	xcb_configure_window(dpy, win, XCB_CONFIG_WINDOW_STACK_MODE, stack_mode);
}

/**
 * @brief Request all of the atoms that howm supports.
 *
 * @param names The names of the atoms to be fetched.
 * @param atoms Where the returned atoms will be stored.
 */
void get_atoms(char **names, xcb_atom_t *atoms)
{
	xcb_intern_atom_reply_t *reply;
	unsigned int i, cnt;

	cnt = LENGTH(atoms);
	xcb_intern_atom_cookie_t cookies[cnt];

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

/**
 * @brief Arrange the windows in a stack, whether that be horizontal or
 * vertical is decided by the current_layout.
 */
void stack(void)
{
	Client *c = NULL;
	bool vert = (cur_layout == VSTACK);
	int span = vert ? screen_height - BAR_HEIGHT :  screen_width;
	int i, n, client_x = 0;
	int client_y = BAR_BOTTOM ? 0 : BAR_HEIGHT;

	n = get_non_tff_count();
	if (n == 1) {
		zoom();
		return;
	}

	/* TODO: Need to take into account when this has remainders. */
	/* TODO: Fix gaps between windows. */
	int client_span = (span / n) - (2 * BORDER_PX);
	DEBUG("STACK")

	if (vert) {
		change_client_geom(head, 0, client_y,
			    screen_width - (2 * BORDER_PX), client_span);
		client_y += (BORDER_PX + (span / n));
	} else {
		change_client_geom(head, client_x, BAR_BOTTOM ? 0 : BAR_HEIGHT,
			    client_span, screen_height - (2 * BORDER_PX) - BAR_HEIGHT);
		client_x += (BORDER_PX + (span / n));
	}

	for (c = head->next, i = 0; i < n - 1; c = c->next, i++) {
		if (vert) {
			change_client_geom(c, GAP, client_y,
				    screen_width - (2 * BORDER_PX),
				    client_span - BORDER_PX);
			client_y += (BORDER_PX + client_span);
		} else {
			change_client_geom(c, client_x, BAR_BOTTOM ? 0 : BAR_HEIGHT,
				    client_span - BORDER_PX,
				    screen_height - (2 * BORDER_PX) - BAR_HEIGHT);
			client_x += (BORDER_PX + client_span);
		}
	}
	draw_clients();
}

/**
 * @brief Count how many clients aren't Transient, Floating or Fullscreen
 *
 * @return The amount of clients in the current workspace that aren't TTF.
 */
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

/**
 * @brief The handler for destroy events.
 *
 * Used when a window sends a destroy event, signalling that it wants to be
 * unmapped. The client that the window belongs to is then removed from the
 * client list for its repective workspace.
 *
 * @param ev The destroy event.
 */
void destroy_event(xcb_generic_event_t *ev)
{
	DEBUG("DESTROY");
	xcb_destroy_notify_event_t *de = (xcb_destroy_notify_event_t *)ev;
	Client *c = find_client_by_win(de->window);
	if (c)
		remove_client(c);
}

/**
 * @brief Remove a client from its workspace client list.
 *
 * @param c The client to be removed.
 */
void remove_client(Client *c)
{
	Client **temp = NULL;
	int w = 1, cw = cur_ws;
	bool found;

	for (found = false; w < WORKSPACES && !found; w++)
		for (temp = &head, select_ws(cw); temp &&
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
	if (cw == w)
		arrange_windows();
	else
		select_ws(cw);
}

/**
 * @brief Print debug information about the current state of howm.
 *
 * This can be parsed by programs such as scripts that will pipe their input
 * into a status bar.
 */
void howm_info(void)
{
	int cw = cur_ws;
	int w, n;
	Client *c;

	for (w = 1; w <= WORKSPACES; w++) {
		for (select_ws(w), c = head, n = 0; c; c = c->next, n++)
			;
		printf("m:%d l:%d n:%d w:%d cw:%d s:%d\n", cur_mode,
		       cur_layout, n, w, cur_ws == cw, cur_state);
	}
	if (cw != w)
		select_ws(cw);
}

/**
 * @brief The event that occurs when the mouse pointer enters a window.
 *
 * @param ev The enter event.
 */
void enter_event(xcb_generic_event_t *ev)
{
	xcb_enter_notify_event_t *ee = (xcb_enter_notify_event_t *)ev;

	DEBUG("enter_event");
	if (FOCUS_MOUSE)
		focus_window(ee->event);
}

/**
 * @brief Move a client down in its client list.
 *
 * @param c The client to be moved.
 */
void move_down(Client *c)
{
	if (!c)
		return;
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

/**
 * @brief Move a client up in its client list.
 *
 * @param c The client to be moved down.
 */
void move_up(Client *c)
{
	if (!c)
		return;
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

/**
 * @brief brief Move focus onto the client next in the client list.
 *
 * @param arg The argument passed from the config file. Note: The argument goes
 * unused.
 */
void focus_next_client(const Arg *arg)
{
	if (!current || !head->next)
		return;
	DEBUG("focus_next");
	update_focused_client(current->next ? current->next : head);
}

/**
 * @brief brief Move focus onto the client previous in the client list.
 *
 * @param arg The argument passed from the config file. Note: The argument goes
 * unused.
 */
void focus_prev_client(const Arg *arg)
{
	if (!current || !head->next)
		return;
	DEBUG("focus_prev");
	prev_foc = current;
	update_focused_client(prev_client(prev_foc));
}

/**
 * @brief Change to a different workspace.
 *
 * @param arg arg->i indicates which workspace howm should change to.
 */
void change_ws(const Arg *arg)
{
	if (arg->i > WORKSPACES || arg->i <= 0 || arg->i == cur_ws)
		return;
	last_ws = cur_ws;
	select_ws(arg->i);
	for (Client *c = head; c; c = c->next)
		xcb_map_window(dpy, c->win);
	select_ws(last_ws);
	for (Client *c = head; c; c = c->next)
		xcb_unmap_window(dpy, c->win);
	select_ws(arg->i);
	arrange_windows();
	update_focused_client(current);
	howm_info();
}

/**
 * @brief Focus the previous workspace.
 *
 * @param arg Unused.
 */
void focus_prev_ws(const Arg *arg)
{
	const Arg a = { .i	= cur_ws < 2 ? WORKSPACES :
				  cur_ws - 1
		      };

	change_ws(&a);
}

/**
 * @brief Focus the last focused workspace.
 *
 * @param arg Unused.
 */
void focus_last_ws(const Arg *arg)
{
	const Arg a = { .i = last_ws };

	change_ws(&a);
}

/**
 * @brief Focus the next workspace.
 *
 * @param arg Unused.
 */
void focus_next_ws(const Arg *arg)
{
	const Arg a = { .i = (cur_ws + 1) % WORKSPACES };

	change_ws(&a);
}

/**
 * @brief Change the layout of the current workspace.
 *
 * @param arg A numerical value (arg->i) representing the layout that should be
 * used.
 */
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

/**
 * @brief Change to the previous layout.
 *
 * @param arg Unused.
 */
void previous_layout(const Arg *arg)
{
	const Arg a = { .i = cur_layout < 1 ? END_LAYOUT - 1 : cur_layout - 1 };

	change_layout(&a);
}

/**
 * @brief Change to the next layout.
 *
 * @param arg Unused.
 */
void next_layout(const Arg *arg)
{
	const Arg a = { .i = (cur_layout + 1) % END_LAYOUT };

	change_layout(&a);
}

/**
 * @brief Change to the last used layout.
 *
 * @param arg Unused.
 */
void last_layout(const Arg *arg)
{
	const Arg a = { .i = prev_layout };

	change_layout(&a);
}

/**
 * @brief Change the mode of howm.
 *
 * Modes should be thought of in the same way as they are in vi. Different
 * modes mean keypresses cause different actions.
 *
 * @param arg arg->i defines which mode to be selected.
 */
void change_mode(const Arg *arg)
{
	if (arg->i >= END_MODES || arg->i == cur_mode)
		return;
	cur_mode = arg->i;
	howm_info();
}

/**
 * @brief An operator that kills an arbitrary amount of clients or workspaces.
 *
 * @param type Whether to kill workspaces or clients.
 * @param cnt How many "things" to kill.
 */
void op_kill(const int type, int cnt)
{
	if (type == WORKSPACE) {
		DEBUGP("Killing %d workspaces.\n", cnt);
		while (cnt > 0) {
			kill_ws((cur_ws + cnt) % WORKSPACES);
			cnt--;
		}
	} else if (type == CLIENT) {
		DEBUGP("Killing %d clients.\n", cnt);
		while (cnt > 0) {
			kill_client();
			cnt--;
		}
	}
}

/**
 * @brief Kills the current client.
 */
void kill_client(void)
{
	if (!current)
		return;
	/* TODO: Kill the window in a nicer way and get it to consistently die. */
	xcb_kill_client(dpy, current->win);
	DEBUG("Killing Client");
	remove_client(current);
}

/**
 * @brief Kills the given workspace.
 *
 * @param ws The workspace to be killed.
 */
void kill_ws(const int ws)
{
	Arg arg = { .i = ws };

	change_ws(&arg);
	while (head)
		kill_client();
}

/**
 * @brief Move workspace/s or client/s down.
 *
 * @param type Whether to move a client or workspace.
 * @param cnt How many "things" to move.
 */
void op_move_down(const int type, int cnt)
{
	move_ws_or_client(type, cnt, false);
}

/**
 * @brief Move workspace/s or client/s up.
 *
 * @param type Whether to move a client or workspace.
 * @param cnt How many "things" to move.
 */
void op_move_up(const int type, int cnt)
{
	move_ws_or_client(type, cnt, true);
}

/**
 * @brief Moves workspace or a client either upwards or down.
 *
 * Moves a single client/workspace or multiple clients/workspaces either up or
 * down. The op_move_* functions server as simple wrappers to this.
 *
 * @param type Whether to move a client or workspace.
 * @param cnt How many "things" to move.
 * @param up Whether to move the "things" up or down. True is up.
 */
void move_ws_or_client(const int type, int cnt, bool up)
{
	if (type == WORKSPACE) {
		if (up)
			for (; cnt > 0; cnt--)
				move_ws_up(correct_ws(cur_ws + cnt - 1));
		else
			for (int i = 0; i < cnt; i++)
				move_ws_down(correct_ws(cur_ws + i));
	} else if (type == CLIENT) {
		if (up) {
			if (current == head)
				return;
			Client *c = prev_client(current);
			/* TODO optimise this by inserting the client only once
			 * and in the correct location.*/
			for (; cnt > 0; move_down(c), cnt--)
				;
		} else {
			if (current == prev_client(head))
				return;
			int cntcopy = cnt;
			Client *c;
			for (c = current; cntcopy > 0; c = next_client(c), cntcopy--)
				;
			for (; cnt > 0; move_up(c), cnt--)
				;
		}
	}
}

/**
 * @brief Moves the current client down.
 *
 * @param arg Unused.
 */
void move_current_down(const Arg *arg)
{
	move_down(current);
}

/**
 * @brief Moves the current client up.
 *
 * @param arg Unused.
 */
void move_current_up(const Arg *arg)
{
	move_up(current);
}

/**
 * @brief Moves a client from one workspace to another.
 *
 * @param c The client to be moved.
 * @param ws The ws that the client should be moved to.
 */
void client_to_ws(Client *c, const int ws)
{
	/* Performed for the current workspace. */
	if (!c || ws == cur_ws)
		return;
	Client *last;
	Client *prev = prev_client(c);
	int cw = cur_ws;
	Arg arg = { .i = ws };
	/* Target workspace. */
	change_ws(&arg);
	last = prev_client(head);
	if (!head)
		head = c;
	else if (last)
		last->next = c;
	else
		head->next = c;

	arg.i = cw;
	/* Current workspace. */
	change_ws(&arg);
	if (c == head || !prev)
		head = next_client(c);
	else
		prev->next = next_client(c);
	c->next = NULL;
	xcb_unmap_window(dpy, c->win);
	update_focused_client(prev_foc);
	if (FOLLOW_MOVE) {
		arg.i = ws;
		change_ws(&arg);
	} else {
		arrange_windows();
	}
}

/**
 * @brief Moves the current client to the workspace passed in through arg.
 *
 * @param arg arg->i is the target workspace.
 */
void current_to_ws(const Arg *arg)
{
	client_to_ws(current, arg->i);
}

/**
 * @brief Gets the number of the previous workspace.
 *
 * Wraps from the lowest workspace to the highest.
 *
 * @param ws The workspace who's previous should be fetched.
 *
 * @return The number of the previous workspace.
 */
int prev_ws(int ws)
{
	return correct_ws(correct_ws(ws) - 1);
}

/**
 * @brief Gets the number of the next workspace.
 *
 * Wraps from the highest workspace to the lowest.
 *
 * @param ws The workspace who's next should be fetched.
 *
 * @return The number of the next workspace.
 */
int next_ws(int ws)
{
	return correct_ws(correct_ws(ws) + 1);
}

/**
 * @brief Correctly wrap a workspace number.
 *
 * This prevents workspace numbers from being greater than WORKSPACES or less
 * than 1.
 *
 * @param ws The value that needs to be corrected.
 *
 * @return A corrected workspace number.
 */
int correct_ws(int ws)
{
	if (ws > WORKSPACES)
		return ws - WORKSPACES;
	else if (ws < 1)
		return ws + WORKSPACES;
	else
		return ws;
}

/**
 * @brief Move the entirety of one workspace to another.
 *
 * Takes every client from one workspace and places them, in original order,
 * onto the end of the destination workspace's client list.
 *
 * @param s_ws The source workspace that clients should be moved from.
 * @param d_ws The target workspace that clients should be moved to.
 */
void move_ws(int s_ws, int d_ws)
{
	/* Source workspace. */
	Arg arg = { .i = s_ws };

	change_ws(&arg);
	while (head)
		/* The destination workspace. */
		client_to_ws(head, d_ws);
	change_ws(&arg);
}

/**
 * @brief Move the entirety of the current workspace to the next workspace
 * down.
 *
 * @param ws The workspace to be moved.
 */
void move_ws_down(int ws)
{
	move_ws(ws, correct_ws(prev_ws(ws)));
}

/**
 * @brief Move the entirety of the current workspace to the next workspace
 * up.
 *
 * @param ws The workspace to be moved.
 */
void move_ws_up(int ws)
{
	move_ws(ws, correct_ws(next_ws(ws)));
}

/**
 * @brief Focus the given window.
 *
 * @param win A window that belongs to a client being managed by howm.
 */
void focus_window(xcb_window_t win)
{
	Client *c = find_client_by_win(win);

	if (c)
		update_focused_client(c);
}

/**
 * @brief Operator function to move the current focus up.
 *
 * @param type Whether to focus on clients or workspaces.
 * @param cnt The number of times to move focus.
 */
void op_focus_up(const int type, int cnt)
{
	while (cnt > 0) {
		if (type == CLIENT)
			focus_next_client(NULL);
		else if (type == WORKSPACE)
			focus_next_ws(NULL);
		else
			return;
		cnt--;
	}
}

/**
 * @brief Operator function to move the current focus down.
 *
 * @param type Whether to focus on clients or workspaces.
 * @param cnt The number of times to move focus.
 */
void op_focus_down(const int type, int cnt)
{
	while (cnt > 0) {
		if (type == CLIENT)
			focus_prev_client(NULL);
		else if (type == WORKSPACE)
			focus_prev_ws(NULL);
		else
			return;
		cnt--;
	}
}

/**
 * @brief Deal with a window's request to change its geometry.
 *
 * @param ev The event sent from the window.
 */
void configure_event(xcb_generic_event_t *ev)
{
	xcb_configure_request_event_t *ce = (xcb_configure_request_event_t *)ev;
	Client *c = find_client_by_win(ce->window);
	unsigned int vals[7], i = 0;

	/* TODO: Need to test whether gaps etc need to be taken into account
	 * here. */
	if (XCB_CONFIG_WINDOW_X & ce->value_mask)
		vals[i++] = ce->x;
	if (XCB_CONFIG_WINDOW_Y & ce->value_mask)
		vals[i++] = ce->y + (BAR_BOTTOM ? 0 : BAR_HEIGHT);
	if (XCB_CONFIG_WINDOW_WIDTH & ce->value_mask)
		vals[i++] = (ce->width < screen_width - BORDER_PX) ? ce->width : screen_width - BORDER_PX;
	if (XCB_CONFIG_WINDOW_HEIGHT & ce->value_mask)
		vals[i++] = (ce->height < screen_height - BORDER_PX) ? ce->height : screen_height - BORDER_PX;
	if (XCB_CONFIG_WINDOW_BORDER_WIDTH & ce->value_mask)
		vals[i++] = ce->border_width;
	if (XCB_CONFIG_WINDOW_SIBLING & ce->value_mask)
		vals[i++] = ce->sibling;
	if (XCB_CONFIG_WINDOW_STACK_MODE & ce->value_mask)
		vals[i++] = ce->stack_mode;
	xcb_configure_window(dpy, ce->window, ce->value_mask, vals);
	arrange_windows();
}

void unmap_event(xcb_generic_event_t *ev)
{
	xcb_unmap_notify_event_t *ue = (xcb_unmap_notify_event_t *)ev;
	Client *c = create_client(ue->window);

	if (c && !ue->event == screen->root)
		remove_client(c);
	howm_info();
}

/**
 * @brief Arrange the client's windows on the screen.
 *
 * This function takes some strain off of the layout handlers by passing the
 * client's dimensions to move_resize. This splits the layout handlers into
 * smaller, more understandable parts.
 */
void draw_clients(void)
{
	Client *c = NULL;
	for (c = head; c; c = c->next)
		move_resize(c->win, true, c->x, c->y, c->w, c->h);
}

/**
 * @brief Change the size and location of a client.
 *
 * @param c The client to be changed.
 * @param x The x coordinate of the client's window.
 * @param y The y coordinate of the client's window.
 * @param w The width of the client's window.
 * @param h The height of the client's window.
 */
void change_client_geom(Client *c, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	c->x = x;
	c->y = y;
	c->w = w;
	c->h = h;
}
