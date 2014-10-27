#ifndef CONFIG_H
#define CONFIG_H

/**
 * @file config.h
 */

/*
 *┌────────────┐
 *│╻ ╻┏━┓╻ ╻┏┳┓│
 *│┣━┫┃ ┃┃╻┃┃┃┃│
 *│╹ ╹┗━┛┗┻┛╹ ╹│
 *└────────────┘
 */

/** The main modifier key. */
#define MODKEY Mod4Mask
/** The modifier key that, when combined with a number, defines a count. */
#define COUNT_MOD Mod1Mask
/** The modifier key that is used for motions and operators. */
#define OTHER_MOD Mod1Mask
/** Number of workspaces. */
#define WORKSPACES 5
/** When moving the mouse over a window, focus on the window? */
#define FOCUS_MOUSE false
/** Clicking a window will focus it. */
#define FOCUS_MOUSE_CLICK true
/** Upon moving a window to a different workspace, move the focus to the
 * workspace? */
#define FOLLOW_MOVE true
/** The size (in pixels) of the useless gaps. */
#define GAP 2
/** Enable debugging output */
#define DEBUG_ENABLE false
/** The size (in pixels) of the border. */
#define BORDER_PX 2
/** The border colour when the window is focused. */
#define BORDER_FOCUS "#70898F"
/** The border colour when the window is unfocused. */
#define BORDER_UNFOCUS "#555555"
/** The border colour of the last focused window.. */
#define BORDER_PREV_FOCUS "#74718E"
/** The border colour when the window is urgent. */
#define BORDER_URGENT "#FF0000"
/** The height of the status bar to be displayed. */
#define BAR_HEIGHT 20
/** Whether the status bar is at the top or bottom of the screen. */
#define BAR_BOTTOM true
/** The amount of pixels that the op_shrink_gap and op_grow_gap change the gap
 * size by. */
#define OP_GAP_SIZE 4
/** Upon converting a window to floating, should it be centered? */
#define CENTER_FLOATING true
/** Draw a gap around a window when in zoom mode. */
#define ZOOM_GAP true
/** The ratio of the size of the master window compared to the screen's size. */
#define MASTER_RATIO 0.7
/** How much detail should be logged. A LOG_LEVEL of INFO will log almost
 * everything, LOG_WARN will log warnings and errors and LOG_ERR will log only
 * errors.
 *
 * LOG_NONE means nothing will be logged.
 *
 * LOG_DEBUG should be used by developers.
 */
#define LOG_LEVEL LOG_INFO
/** The workspace that should be focused upon startup. */
#define DEFAULT_WORKSPACE 1
/** The minimum width of a floating window that is spawned, if it doesn't
 * respond to geometry requests in a satisfactory manner. */
#define FLOAT_SPAWN_WIDTH 500
/** The minimum height of a floating window that is spawned, if it doesn't
 * respond to geometry requests in a satisfactory manner. */
#define FLOAT_SPAWN_HEIGHT 500
/** The path at which the howm binary (or script that started howm) is stored
 * at. This is used for restarts. */
#define HOWM_PATH "/usr/bin/howm"
/** The amount of client lists that can be stored in the register before
 * needing to be pasted back. */
#define DELETE_REGISTER_SIZE 5
/** The height of the floating scratchpad window. */
#define SCRATCHPAD_HEIGHT 500
/** The width of the floating scratchpad window. */
#define SCRATCHPAD_WIDTH 500
/** The path that howm's unix socket is at. */
#define SOCK_PATH "/tmp/howm"
/** The size of the socket buffer. */
#define IPC_BUF_SIZE 1024

static const char * const term_cmd[] = {"urxvt", NULL};
static const char * const dmenu_cmd[] = {"dmenu_run", "-i", "-b",
		    "-nb", "#70898f", "-nf", "black",
		    "-sf", "#74718e", NULL};

/* Rules that are applied to clients as they are spawned. */
static const Rule rules[] = {
	/* Class, WS, follow, float, fullscreen */
	{"dwb", 3, false, false, false},
	{"mpv", 5, false, false, false}
};

/** @brief The standard key map, feel free to change them.
 *
 * In the form:
 *
 * {Modifier, Mode, Key, Command, Args}
 */
static const Key keys[] = {
	{ MODKEY, NORMAL, XK_Return, spawn, {.cmd = term_cmd} },
	{ MODKEY, NORMAL, XK_d, spawn, {.cmd = dmenu_cmd} },

	{ MODKEY, NORMAL, XK_v, change_layout, {.i = VSTACK} },
	{ MODKEY, NORMAL, XK_h, change_layout, {.i = HSTACK} },
	{ MODKEY, NORMAL, XK_g, change_layout, {.i = GRID} },
	{ MODKEY, NORMAL, XK_z, change_layout, {.i = ZOOM} },
	{ MODKEY, NORMAL, XK_n, next_layout, {NULL} },
	{ MODKEY, NORMAL, XK_l, last_layout, {NULL} },
	{ MODKEY | ShiftMask, NORMAL, XK_n, previous_layout, {NULL} },
	{ MODKEY, NORMAL, XK_f, change_mode, {.i = FOCUS} },
	{ MODKEY | ShiftMask, NORMAL, XK_f, change_mode, {.i = FLOATING} },
	{ MODKEY, NORMAL, XK_space, toggle_float, {NULL} },
	{ MODKEY, NORMAL, XK_Delete, quit_howm, {.i = EXIT_SUCCESS} },
	{ MODKEY | ShiftMask, NORMAL, XK_Delete, quit_howm, {.i = EXIT_FAILURE} },
	{ MODKEY, NORMAL, XK_BackSpace, restart_howm, {NULL} },
	{ MODKEY, NORMAL, XK_m, resize_master, {.i = 5} },
	{ MODKEY | ShiftMask, NORMAL, XK_m, resize_master, {.i = -5} },
	{ MODKEY, NORMAL, XK_b, toggle_bar, {NULL} },
	{ MODKEY, NORMAL, XK_period, replay, {NULL} },
	{ MODKEY, NORMAL, XK_p, paste, {NULL} },
	{ MODKEY, NORMAL, XK_q, send_to_scratchpad, {NULL} },
	{ MODKEY | ShiftMask, NORMAL, XK_q, get_from_scratchpad, {NULL} },

	{ MODKEY | ShiftMask, FLOATING, XK_k, resize_float_height, {.i = -10} },
	{ MODKEY | ShiftMask, FLOATING, XK_j, resize_float_height, {.i = 10} },
	{ MODKEY | ShiftMask, FLOATING, XK_h, resize_float_width, {.i = -10} },
	{ MODKEY | ShiftMask, FLOATING, XK_l, resize_float_width, {.i = 10} },
	{ MODKEY, FLOATING, XK_j, move_float_y, {.i = 10} },
	{ MODKEY, FLOATING, XK_k, move_float_y, {.i = -10} },
	{ MODKEY, FLOATING, XK_h, move_float_x, {.i = -10} },
	{ MODKEY, FLOATING, XK_l, move_float_x, {.i = 10} },
	{ MODKEY, FLOATING, XK_y, teleport_client, {.i = TOP_LEFT} },
	{ MODKEY, FLOATING, XK_u, teleport_client, {.i = TOP_CENTER} },
	{ MODKEY, FLOATING, XK_i, teleport_client, {.i = TOP_RIGHT} },
	{ MODKEY, FLOATING, XK_space , teleport_client, {.i = CENTER} },
	{ MODKEY, FLOATING, XK_b, teleport_client, {.i = BOTTOM_LEFT} },
	{ MODKEY, FLOATING, XK_n, teleport_client, {.i = BOTTOM_CENTER} },
	{ MODKEY, FLOATING, XK_m, teleport_client, {.i = BOTTOM_RIGHT} },
	{ MODKEY, FLOATING, XK_Escape, change_mode, {.i = NORMAL} },
	{ MODKEY, FLOATING, XK_f, change_mode, {.i = FOCUS} },

	{ MODKEY, FOCUS, XK_space, toggle_fullscreen, {NULL} },
	{ MODKEY, FOCUS, XK_Tab, focus_urgent, {NULL} },
	{ MODKEY, FOCUS, XK_k, focus_prev_client, {NULL} },
	{ MODKEY, FOCUS, XK_j, focus_next_client, {NULL} },
	{ MODKEY | ShiftMask, FOCUS, XK_k, move_current_up, {NULL} },
	{ MODKEY | ShiftMask, FOCUS, XK_j, move_current_down, {NULL} },
	{ MODKEY, FOCUS, XK_Escape, change_mode, {.i = NORMAL} },
	{ MODKEY, FOCUS, XK_u, focus_last_ws, {NULL} },
	{ MODKEY, FOCUS, XK_l, focus_next_ws, {NULL} },
	{ MODKEY, FOCUS, XK_h, focus_prev_ws, {NULL} },
	{ MODKEY, FOCUS, XK_m, make_master, {NULL} },
	{ MODKEY, FOCUS, XK_1, change_ws, {.i = 1} },
	{ MODKEY, FOCUS, XK_2, change_ws, {.i = 2} },
	{ MODKEY, FOCUS, XK_3, change_ws, {.i = 3} },
	{ MODKEY, FOCUS, XK_4, change_ws, {.i = 4} },
	{ MODKEY, FOCUS, XK_5, change_ws, {.i = 5} },
	{ MODKEY | ShiftMask, FOCUS, XK_1, current_to_ws, {.i = 1} },
	{ MODKEY | ShiftMask, FOCUS, XK_2, current_to_ws, {.i = 2} },
	{ MODKEY | ShiftMask, FOCUS, XK_3, current_to_ws, {.i = 3} },
	{ MODKEY | ShiftMask, FOCUS, XK_4, current_to_ws, {.i = 4} },
	{ MODKEY | ShiftMask, FOCUS, XK_5, current_to_ws, {.i = 5} },
	{ MODKEY | ShiftMask, FOCUS, XK_f, change_mode, {.i = FLOATING} }
};

/**
 * @brief The operations to be performed on a target. All functions that are
 * operators begin with op_*.
 *
 * Vim's built in help is useful for understanding this:
 *
 * :help operators
 */
static const Operator operators[] = {
	{OTHER_MOD, XK_q, NORMAL, op_kill},
	{OTHER_MOD, XK_j, NORMAL, op_move_down},
	{OTHER_MOD, XK_k, NORMAL, op_move_up},
	{OTHER_MOD, XK_g, NORMAL, op_shrink_gaps},
	{OTHER_MOD | ShiftMask, XK_g, NORMAL, op_grow_gaps},
	{OTHER_MOD, XK_d, NORMAL, op_cut},
	{OTHER_MOD, XK_j, FOCUS, op_focus_down},
	{OTHER_MOD, XK_k, FOCUS, op_focus_up}
};

/**
 * @brief Motions can be used to show that an operation should be performed on
 * a certain target- such as a client.
 *
 * For example:
 *
 * q4c (QUIT, 4, Clients)
 * q2w (QUIT, 2, Workspaces)
 *
 */
static const Motion motions[] = {
	{OTHER_MOD, XK_c, CLIENT},
	{OTHER_MOD | ShiftMask, XK_c, CLIENT},
	{OTHER_MOD, XK_w, WORKSPACE},
	{OTHER_MOD | ShiftMask, XK_w, WORKSPACE}
};

/**
 * @brief Workspaces and their default layout.
 *
 * Note: The first item is NULL as workspaces are indexed from 1.
 */
static Workspace wss[] = {
	{0, 0, 0, 0, 0, NULL, NULL, NULL},
	{.layout = HSTACK, .gap = GAP, .master_ratio = 0.6, .bar_height = BAR_HEIGHT},
	{.layout = HSTACK, .gap = GAP, .master_ratio = 0.6, .bar_height = BAR_HEIGHT},
	{.layout = HSTACK, .gap = GAP, .master_ratio = 0.6, .bar_height = BAR_HEIGHT},
	{.layout = HSTACK, .gap = GAP, .master_ratio = 0.6, .bar_height = BAR_HEIGHT},
	{.layout = HSTACK, .gap = GAP, .master_ratio = 0.6, .bar_height = BAR_HEIGHT}
};

_Static_assert(WORKSPACES >= 1, "WORKSPACES must be at least 1.");
_Static_assert(DEFAULT_WORKSPACE > 0 && DEFAULT_WORKSPACE <= WORKSPACES, "DEFAULT_WORKSPACE must be between 1 and WORKSPACES.");
_Static_assert(GAP >= 0, "GAP can't be negative.");
_Static_assert(BORDER_PX >= 0, "BORDER_PX can't be negative.");
_Static_assert(OP_GAP_SIZE >= 0, "OP_GAP_SIZE can't be negative.");
_Static_assert(BAR_HEIGHT >= 0, "BAR_HEIGHT can't be negative.");
_Static_assert(FLOAT_SPAWN_HEIGHT >= 0, "FLOAT_SPAWN_HEIGHT can't be negative.");
_Static_assert(FLOAT_SPAWN_WIDTH >= 0, "FLOAT_SPAWN_WIDTH can't be negative.");
_Static_assert(LENGTH(wss) == WORKSPACES + 1, "wss must contain one more workspace than WORKSPACES.");
_Static_assert(SCRATCHPAD_WIDTH >= 0, "SCRATCHPAD_WIDTH can't be negative.");
_Static_assert(SCRATCHPAD_HEIGHT >= 0, "SCRATCHPAD_HEIGHT can't be negative.");
#endif
