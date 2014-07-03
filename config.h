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

static const char * const term_cmd[] = {"urxvt", "-e", "sleep", "300", NULL};
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
	{.layout = HSTACK},
	{.layout = HSTACK},
	{.layout = HSTACK}
};

#endif
