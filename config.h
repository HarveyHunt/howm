#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include <stdlib.h>
#include <X11/keysym.h>
#include <X11/X.h>
#include "op.h"
#include "command.h"
#include "layout.h"
#include "client.h"
#include "workspace.h"

/**
 * @file config.h
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

/** The main modifier key. */
#define MODKEY Mod4Mask
/** The modifier key that, when combined with a number, defines a count. */
#define COUNT_MOD Mod1Mask
/** The modifier key that is used for motions and operators. */
#define OTHER_MOD Mod1Mask
/** Enable debugging output */
#define DEBUG_ENABLE false
/** How much detail should be logged. A LOG_LEVEL of INFO will log almost
 * everything, LOG_WARN will log warnings and errors and LOG_ERR will log only
 * errors.
 *
 * LOG_NONE means nothing will be logged.
 *
 * LOG_DEBUG should be used by developers.
 */
#define LOG_LEVEL LOG_DEBUG

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
static Key keys[] = {
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
#endif
