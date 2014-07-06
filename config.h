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
#define WORKSPACES 3
/** When moving the mouse over a window, focus on the window? */
#define FOCUS_MOUSE true
/** Clicking a window will focus it. */
#define FOCUS_MOUSE_CLICK true
/** Upon spawning a new window, move the mouse to the new window? */
#define FOLLOW_SPAWN true
/** The size (in pixels) of the useless gaps. */
#define GAP 0
/** Enable debugging output */
#define DEBUG_ENABLE 1
/** The size (in pixels) of the border. */
#define BORDER_PX 2
/** The border colour when the window is focused. */
#define BORDER_FOCUS "#70898F"
/** The border colour when the window is unfocused. */
#define BORDER_UNFOCUS "#555555"
/** The border colour of the last focused window.. */
#define BORDER_PREV_FOCUS "#74718E"
/** The height of the status bar to be displayed. */
#define BAR_HEIGHT 20
/** Whether the status bar is at the top or bottom of the screen. */
#define BAR_BOTTOM true
/** The amount of pixels that the op_shrink_gap and op_grow_gap change the gap
 * size by. */
#define OP_GAP_SIZE 2
/** Upon converting a window to floating, should it be centered? */
#define CENTER_FLOATING true
/** Draw a gap around a window when in zoom mode. */
#define ZOOM_GAP true
/** The ratio of the size of the master window compared to the screen's size. */
#define MASTER_RATIO 0.7

static const char * const term_cmd[] = {"urxvt", NULL};
static const char * const dmenu_cmd[] = {"dmenu_run", "-i", "-h", "21", "-b",
		    "-nb", "#70898f", "-nf", "black",
		    "-sf", "#74718e", "-fn",
		    "'Droid Sans Mono-10'", NULL};

/** @brief The standard key map, feel free to change them.
 *
 * In the form:
 *
 * {Modifier, Mode, Key, Command, Args}
 */
static const Key keys[] = {
	{ MODKEY, NORMAL, XK_Return, spawn, {.cmd = term_cmd} },
	{ MODKEY, NORMAL, XK_d, spawn, {.cmd = dmenu_cmd} },

	{ MODKEY, NORMAL, XK_s, change_layout, {.i = VSTACK} },
	{ MODKEY, NORMAL, XK_g, change_layout, {.i = GRID} },
	{ MODKEY, NORMAL, XK_z, change_layout, {.i = ZOOM} },
	{ MODKEY, NORMAL, XK_n, next_layout, {NULL} },
	{ MODKEY | ShiftMask, NORMAL, XK_n, previous_layout, {NULL} },
	{ MODKEY, NORMAL, XK_f, change_mode, {.i = FOCUS} },
	{ MODKEY | ShiftMask, NORMAL, XK_f, change_mode, {.i = FLOATING} },
	{ MODKEY, NORMAL, XK_space, toggle_float, {NULL} },
	{ MODKEY, NORMAL, XK_Delete, quit, {.i = EXIT_SUCCESS} },
	{ MODKEY | ShiftMask, NORMAL, XK_Delete, quit, {EXIT_FAILURE} },
	{ MODKEY, NORMAL, XK_m, resize_master, {.i = 5} },
	{ MODKEY | ShiftMask, NORMAL, XK_m, resize_master, {.i = -5} },
	{ MODKEY, NORMAL, XK_b, toggle_bar, {NULL} },

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

	{ MODKEY, FOCUS, XK_k, focus_prev_client, {NULL} },
	{ MODKEY, FOCUS, XK_j, focus_next_client, {NULL} },
	{ MODKEY | ShiftMask, FOCUS, XK_k, move_current_up, {NULL} },
	{ MODKEY | ShiftMask, FOCUS, XK_j, move_current_down, {NULL} },
	{ MODKEY, FOCUS, XK_Escape, change_mode, {.i = NORMAL} },
	{ MODKEY, FOCUS, XK_l, focus_next_ws, {NULL} },
	{ MODKEY, FOCUS, XK_h, focus_prev_ws, {NULL} },
	{ MODKEY, FOCUS, XK_1, change_ws, {.i = 1} },
	{ MODKEY, FOCUS, XK_2, change_ws, {.i = 2} },
	{ MODKEY, FOCUS, XK_3, change_ws, {.i = 3} },
	{ MODKEY | ShiftMask, FOCUS, XK_1, current_to_ws, {.i = 1} },
	{ MODKEY | ShiftMask, FOCUS, XK_2, current_to_ws, {.i = 2} },
	{ MODKEY | ShiftMask, FOCUS, XK_3, current_to_ws, {.i = 3} }
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
	{OTHER_MOD, XK_g, NORMAL, op_shrink_gaps},
	{OTHER_MOD | ShiftMask, XK_g, NORMAL, op_grow_gaps},
	{OTHER_MOD, XK_k, NORMAL, op_move_up},
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
	{OTHER_MOD, XK_w, WORKSPACE}
};

/**
 * @brief Workspaces and their default layout.
 *
 * Note: The first item is NULL as workspaces are indexed from 1.
 */
static Workspace workspaces[] = {
	{NULL},
	{.layout = HSTACK, .gap = 4, .master_ratio = 0.6, .bar_height = BAR_HEIGHT},
	{.layout = HSTACK, .gap = 4, .master_ratio = 0.6, .bar_height = BAR_HEIGHT},
	{.layout = HSTACK, .gap = 4, .master_ratio = 0.6, .bar_height = BAR_HEIGHT}
};

#endif
