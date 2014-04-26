#ifndef CONFIG_H
#define CONFIG_H

#define MODKEY Mod4Mask
#define WORKSPACES 3
#define FOCUS_MOUSE 0
#define FOLLOW_MOVE 0
#define GAP 0
#define DEBUG_ENABLE 1
#define ZOOM_GAP 0
#define ZOOM_BORDER 0
#define BORDER_PX 2
#define BORDER_FOCUS "#FF00FF"
#define BORDER_UNFOCUS "#00FF00"

static const char *term_cmd[] = {"urxvt", "-e", "sleep", "300", NULL};
static const char *dmenu_cmd[] = {"dmenu_run", "-i", "-h", "21", "-b",
					"-nb", "#70898f", "-nf", "black",
					"-sf", "#74718e", "-fn",
					"'Droid Sans Mono-10'"};

static int count_mod = MODKEY;

static const Key keys[] = {
	{MODKEY, NORMAL, XK_Return, spawn, {.cmd = term_cmd} },
	{MODKEY, NORMAL, XK_r, spawn, {.cmd = dmenu_cmd} },

	{MODKEY, NORMAL, XK_s, change_layout, {.i = VSTACK} },
	{MODKEY, NORMAL, XK_g, change_layout, {.i = GRID} },
	{MODKEY, NORMAL, XK_s, change_layout, {.i = ZOOM} },
	{MODKEY, NORMAL, XK_n, next_layout, {} },
	{MODKEY | ShiftMask, NORMAL, XK_n, previous_layout, {} },
	{MODKEY, NORMAL, XK_f, change_mode, {.i = FOCUS} },

	{MODKEY, FOCUS, XK_k, focus_prev_client, {} },
	{MODKEY, FOCUS, XK_j, focus_next_client, {} },
	{MODKEY | ShiftMask, FOCUS, XK_k, move_current_up, {} },
	{MODKEY | ShiftMask, FOCUS, XK_j, move_current_down, {} },
	{MODKEY, FOCUS, XK_Escape, change_mode, {.i = NORMAL} },

	{MODKEY, FOCUS, XK_space, focus_next_ws, {} },
	{MODKEY | ShiftMask, FOCUS, XK_space, focus_prev_ws, {} },
	{MODKEY, FOCUS, XK_1, change_ws, {.i = 1} },
    {MODKEY, FOCUS, XK_2, change_ws, {.i = 2} },
    {MODKEY, FOCUS, XK_3, change_ws, {.i = 3} },
    {MODKEY | ShiftMask, FOCUS, XK_1, current_to_ws, {.i = 1} },
    {MODKEY | ShiftMask, FOCUS, XK_2, current_to_ws, {.i = 2} },
    {MODKEY | ShiftMask, FOCUS, XK_3, current_to_ws, {.i = 3} }
};

static const Operator operators[] = {
	{MODKEY, XK_q, op_kill},
    {MODKEY, XK_j, op_move_down},
    {MODKEY, XK_k, op_move_up}
};

static const Motion motions[] = {
	{MODKEY, XK_c, CLIENT},
	{MODKEY, XK_w, WORKSPACE}
};

static Workspace workspaces[] = {
    {NULL},
	{.layout = HSTACK},
	{.layout = HSTACK},
	{.layout = HSTACK}
};

#endif
