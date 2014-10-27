#include <err.h>
#include <errno.h>
#include <sys/select.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <X11/keysym.h>
#include <X11/X.h>
#include <xcb/xcb.h>
#include <xcb/xcb_icccm.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xcb_ewmh.h>

/**
 * @file howm.c
 *
 * @author Harvey Hunt
 *
 * @date 2014
 *
 * @brief howm
 */

/*
 *┌────────────┐
 *│╻ ╻┏━┓╻ ╻┏┳┓│
 *│┣━┫┃ ┃┃╻┃┃┃┃│
 *│╹ ╹┗━┛┗┻┛╹ ╹│
 *└────────────┘
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
#define FFT(c) (c->is_transient || c->is_floating || c->is_fullscreen)
/** Supresses the unused variable compiler warnings. */
#define UNUSED(x) (void)(x)
/** Determine which file descriptor is the largest and add one to it. */
#define MAX_FD(x, y) ((x) > (y) ? (x + 1) : (y + 1))

/** The remove action for a WM_STATE request. */
#define _NET_WM_STATE_REMOVE 0
/** The add action for a WM_STATE request. */
#define _NET_WM_STATE_ADD 1
/** The toggle action for a WM_STATE request. */
#define _NET_WM_STATE_TOGGLE 2

/**
 * @brief Represents an argument.
 *
 * Used to hold data that is sent as a parameter to a function when called as a
 * result of a keypress.
 */
typedef union {
	const char * const * const cmd; /**< Represents a command that will be called by a shell.  */
	int i; /**< Usually used for specifying workspaces or clients. */
} Arg;

/**
 * @brief Represents a key.
 *
 * Holds information relative to a key, such as keysym and the mode during
 * which the keypress can be seen as valid.
 */
typedef struct {
	int mod; /**< The mask of the modifiers pressed. */
	unsigned int mode; /**< The mode within which this keypress is valid. */
	xcb_keysym_t sym;  /**< The keysym of the pressed key. */
	void (*func)(const Arg *); /**< The function to be called when this key is pressed. */
	const Arg arg; /**< The argument passed to the above function. */
} Key;

/**
 * @brief Represents a rule that is applied to a client upon it starting.
 */
typedef struct {
	const char *class; /**<	The class or name of the client. */
	int ws; /**<  The workspace that the client should be spawned
				on (0 means current workspace). */
	bool follow; /**< If the client is spawned on another ws, shall we follow? */
	bool is_floating; /**< Spawn the client in a floating state? */
	bool is_fullscreen; /**< Spawn the client in a fullscreen state? */
} Rule;

/**
 * @brief Represents an operator.
 *
 * Operators perform an action upon one or more targets (identified by
 * motions).
 */
typedef struct {
	int mod; /**< The mask of the modifiers pressed. */
	xcb_keysym_t sym; /**< The keysym of the pressed key. */
	unsigned int mode; /**< The mode within which this keypress is valid. */
	void (*func)(const unsigned int type, const int cnt); /**< The function to be
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
	int mod; /**< The mask of the modifiers pressed. */
	xcb_keysym_t sym; /**< The keysym of the pressed key. */
	unsigned int type; /**< Represents whether the motion is for clients, WS etc. */
} Motion;

/**
 * @brief Represents a button.
 *
 * Allows the mapping of a button to a function, as is done with the Key struct
 * for keys.
 */
typedef struct {
	int mod; /**< The mask of the modifiers pressed.  */
	short int button; /**< The button that was pressed. */
	void (*func)(const Arg *); /**< The function to be called when the
					* button is pressed. */
	const Arg arg; /**< The argument passed to the above function. */
} Button;

/**
 * @brief Represents a client that is being handled by howm.
 *
 * All the attributes that are needed by howm for a client are stored here.
 */
typedef struct Client {
	struct Client *next; /**< Clients are stored in a linked list-
					* this represents the client after this one. */
	bool is_fullscreen; /**< Is the client fullscreen? */
	bool is_floating; /**< Is the client floating? */
	bool is_transient; /**< Is the client transient?
					* Defined at: http://standards.freedesktop.org/wm-spec/wm-spec-latest.html*/
	bool is_urgent; /**< This is set by a client that wants focus for some reason. */
	xcb_window_t win; /**< The window that this client represents. */
	uint16_t x; /**< The x coordinate of the client. */
	uint16_t y; /**< The y coordinate of the client. */
	uint16_t w; /**< The width of the client. */
	uint16_t h; /**< The height of the client. */
	uint16_t gap; /**< The size of the useless gap between this client and
			the others. */
} Client;

/**
 * @brief Represents a workspace, which stores clients.
 *
 * Clients are stored as a linked list. Changing to a different workspace will
 * cause different clients to be rendered on the screen.
 */
typedef struct {
	int layout; /**< The current layout of the WS, as defined in the
				* layout enum. */
	int client_cnt; /**< The amount of clients on this workspace. */
	uint16_t gap; /**< The size of the useless gap between windows for this workspace. */
	float master_ratio; /**< The ratio of the size of the master window
				 compared to the screen's size. */
	uint16_t bar_height; /**< The height of the space left for a bar. Stored
			      here so it can be toggled per ws. */
	Client *head; /**< The start of the linked list. */
	Client *prev_foc; /**< The last focused client. This is seperate to
				* the linked list structure. */
	Client *current; /**< The client that is currently in focus. */
} Workspace;

typedef struct {
	char *name; /**< The function's name. */
	void (*func)(const Arg *); /**< The function to be called when a command
				     comes in from the socket. */
	void (*operator)(const unsigned int type, const int cnt); /**< The
			operator to be called when a command comes in from
			the socket. */
	int argc; /**< The amount of args this command expects. */
	int arg_type; /**< The argument's type for commands that use the union Arg. */
} Command;

/**
 * @brief Represents the last command (and its arguments) or the last
 * combination of operator, count and motion (ocm).
 */
struct replay_state {
	void (*last_op)(const unsigned int type, int cnt); /**< The last operator to be called. */
	void (*last_cmd)(const Arg *arg); /**< The last command to be called. */
	const Arg *last_arg; /**< The last argument, passed to the last command. */
	unsigned int last_type; /**< The value determine by the last motion
				(workspace, client etc).*/
	int last_cnt; /**< The last count passed to the last operator function. */
};

/**
 * @brief Represents a stack. This stack is going to hold linked lists of
 * clients. An example of the stack is below:
 *
 * TOP
 * ==========
 * c1->c2->c3->NULL
 * ==========
 * c1->NULL
 * ==========
 * c1->c2->c3->NULL
 * ==========
 * BOTTOM
 *
 */
struct stack {
	int size; /**< The amount of items in the stack. */
	Client **contents; /**< The contents is an array of linked lists. Storage
			is malloced later as we don't know the size yet.*/
};


/* Modes */
static void change_mode(const Arg *arg);

enum layouts { ZOOM, GRID, HSTACK, VSTACK, END_LAYOUT };
enum states { OPERATOR_STATE, COUNT_STATE, MOTION_STATE, END_STATE };
enum modes { NORMAL, FOCUS, FLOATING, END_MODES };
enum motions { CLIENT, WORKSPACE };
enum net_atom_enum { NET_WM_STATE_FULLSCREEN, NET_SUPPORTED, NET_WM_STATE,
	NET_ACTIVE_WINDOW };
enum wm_atom_enum { WM_DELETE_WINDOW, WM_PROTOCOLS };
enum teleport_locations { TOP_LEFT, TOP_CENTER, TOP_RIGHT, CENTER, BOTTOM_LEFT, BOTTOM_CENTER, BOTTOM_RIGHT };
enum ipc_errs { IPC_ERR_NONE, IPC_ERR_SYNTAX, IPC_ERR_ALLOC, IPC_ERR_NO_CMD, IPC_ERR_TOO_MANY_ARGS,
	IPC_ERR_TOO_FEW_ARGS, IPC_ERR_ARG_NOT_INT, IPC_ERR_ARG_TOO_LARGE };
enum arg_types {TYPE_IGNORE, TYPE_INT, TYPE_CMD};

#include "config.h"

static void (*operator_func)(const unsigned int type, int cnt);

static Client *scratchpad;
static struct stack del_reg;
static xcb_connection_t *dpy;
static char *WM_ATOM_NAMES[] = { "WM_DELETE_WINDOW", "WM_PROTOCOLS" };
static xcb_atom_t wm_atoms[LENGTH(WM_ATOM_NAMES)];
static xcb_screen_t *screen;
static xcb_ewmh_connection_t *ewmh;
static int numlockmask, retval, last_ws, prev_layout, cw = DEFAULT_WORKSPACE;
static uint32_t border_focus, border_unfocus, border_prev_focus, border_urgent;
static unsigned int cur_mode, cur_state = OPERATOR_STATE, cur_cnt = 1;
static uint16_t screen_height, screen_width;
static bool running = true, restart;

static struct replay_state rep_state;

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
	if (!screen)
		log_err("Can't acquire the default screen.");
	screen_height = screen->height_in_pixels;
	screen_width = screen->width_in_pixels;

	log_info("Screen's height is: %d", screen_height);
	log_info("Screen's width is: %d", screen_width);

	grab_keys();

	get_atoms(WM_ATOM_NAMES, wm_atoms);

	setup_ewmh();

	border_focus = get_colour(BORDER_FOCUS);
	border_unfocus = get_colour(BORDER_UNFOCUS);
	border_prev_focus = get_colour(BORDER_PREV_FOCUS);
	border_urgent = get_colour(BORDER_URGENT);
	stack_init(&del_reg);

	howm_info();
}

/**
 * @brief The code that glues howm together...
 */
int main(int argc, char *argv[])
{
	UNUSED(argc);
	UNUSED(argv);
	fd_set descs;
	int sock_fd, dpy_fd, cmd_fd, ret;
	ssize_t n;
	xcb_generic_event_t *ev;
	char *data = calloc(IPC_BUF_SIZE, sizeof(char));

	if (!data) {
		log_err("Can't allocate memory for socket buffer.");
		exit(EXIT_FAILURE);
	}

	dpy = xcb_connect(NULL, NULL);
	if (xcb_connection_has_error(dpy)) {
		log_err("Can't open X connection");
		exit(EXIT_FAILURE);
	}
	sock_fd = ipc_init();
	setup();
	check_other_wm();
	dpy_fd = xcb_get_file_descriptor(dpy);
	while (running) {
		if (!xcb_flush(dpy))
			log_err("Failed to flush X connection");

		FD_ZERO(&descs);
		FD_SET(dpy_fd, &descs);
		FD_SET(sock_fd, &descs);

		if (select(MAX_FD(dpy_fd, sock_fd), &descs, NULL, NULL, NULL) > 0) {
			if (FD_ISSET(sock_fd, &descs)) {
				cmd_fd = accept(sock_fd, NULL, 0);
				if (cmd_fd == -1) {
					log_err("Failed to accept connection");
					continue;
				}
				n = read(cmd_fd, data, IPC_BUF_SIZE - 1);
				if (n > 0) {
					data[n] = '\0';
					ret = ipc_process_cmd(data, n);
					if (write(cmd_fd, &ret, sizeof(int)) == -1)
						log_err("Unable to send response. errno: %d", errno);
					close(cmd_fd);
				}
			}
			if (FD_ISSET(dpy_fd, &descs)) {
				while ((ev = xcb_poll_for_event(dpy)) != NULL) {
					if (ev && handler[ev->response_type & ~0x80])
						handler[ev->response_type & ~0x80](ev);
					else
						log_debug("Unimplemented event: %d", ev->response_type & ~0x80);
					free(ev);
				}
			}
			if (xcb_connection_has_error(dpy)) {
				log_err("XCB connection encountered an error.");
				running = false;
			}
		}
	}

	cleanup();
	xcb_disconnect(dpy);
	close(sock_fd);
	free(data);

	if (!running && !restart) {
		return retval;
	} else if (!running && restart) {
		char *const argv[] = {HOWM_PATH, NULL};

		execv(argv[0], argv);
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}

/**
 * @brief Print debug information about the current state of howm.
 *
 * This can be parsed by programs such as scripts that will pipe their input
 * into a status bar.
 */
void howm_info(void)
{
	unsigned int w = 0;
#if DEBUG_ENABLE
	for (w = 1; w <= WORKSPACES; w++) {
		fprintf(stdout, "%u:%d:%u:%u:%u\n", cur_mode,
		       wss[w].layout, w, cur_state, wss[w].client_cnt);
	}
	fflush(stdout);
#else
	UNUSED(w);
	fprintf(stdout, "%u:%d:%u:%u:%u\n", cur_mode,
		wss[cw].layout, cw, cur_state, wss[cw].client_cnt);
	fflush(stdout);
#endif
}

/**
 * @brief Quit howm and set the return value.
 *
 * @param arg The return value that howm will send.
 */
static void quit_howm(const Arg *arg)
{
	log_warn("Quitting");
	retval = arg->i;
	running = false;
}


/**
 * @brief Cleanup howm's resources.
 *
 * Delete all of the windows that have been created, remove button and key
 * grabs and remove pointer focus.
 */
static void cleanup(void)
{
	xcb_window_t *w;
	xcb_query_tree_reply_t *q;
	uint16_t i;

	log_warn("Cleaning up");
	xcb_ungrab_key(dpy, XCB_GRAB_ANY, screen->root, XCB_MOD_MASK_ANY);

	q = xcb_query_tree_reply(dpy, xcb_query_tree(dpy, screen->root), 0);
	if (q) {
		w = xcb_query_tree_children(q);
		for (i = 0; i != q->children_len; ++i)
			delete_win(w[i]);
	free(q);
	}
	xcb_set_input_focus(dpy, XCB_INPUT_FOCUS_POINTER_ROOT, screen->root,
			XCB_CURRENT_TIME);
	xcb_ewmh_connection_wipe(ewmh);
	if (ewmh)
		free(ewmh);
	stack_free(&del_reg);
}

/**
 * @brief Restart howm.
 *
 * @param arg Unused.
 */
static void restart_howm(const Arg *arg)
{
	UNUSED(arg);
	log_warn("Restarting.");
	running = false;
	restart = true;
}
