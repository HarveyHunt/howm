#ifndef CONFIG_H
#define CONFIG_H

#define MODKEY Mod4Mask
#define WORKSPACES 3
#define FOCUS_MOUSE 0
#define GAP 0
#define DEBUG_ENABLE 1
#define ZOOM_GAP 0
#define ZOOM_BORDER 0
#define BORDER_PX 2
#define BORDER_FOCUS "#FF00FF"
#define BORDER_UNFOCUS "#00FF00"

static const char *term_cmd[] = {"urxvt", "-e", "sleep", "100", NULL};
static const char *dmenu_cmd[] = {"dmenu_run", "-i", "-h", "21", "-b",
					"-nb", "#70898f", "-nf", "black",
					"-sf", "#74718e", "-fn",
					"'Droid Sans Mono-10'"};

static const Key keys[] = {
	{MODKEY, XK_Return, spawn, {.cmd = term_cmd} },
	{MODKEY, XK_r, spawn, {.cmd = dmenu_cmd} },

	{MODKEY, XK_s, change_layout, {.i = VSTACK} },
	{MODKEY | ShiftMask, XK_s, change_layout, {.i = HSTACK} },
	{MODKEY, XK_g, change_layout, {.i = GRID} },
	{MODKEY, XK_s, change_layout, {.i = ZOOM} },
	{MODKEY, XK_f, next_layout, {} },
	{MODKEY, XK_d, previous_layout, {} },

	{MODKEY, XK_k, focus_prev, {} },
	{MODKEY, XK_j, focus_next, {} },
	{MODKEY | ShiftMask, XK_k, move_up, {} },
	{MODKEY | ShiftMask, XK_j, move_down, {} },

	{MODKEY, XK_space, next_workspace, {} },
	{MODKEY | ShiftMask, XK_space, previous_workspace, {} },

	{MODKEY, XK_1, change_workspace, {.i = 0} },
	{MODKEY, XK_2, change_workspace, {.i = 1} },
	{MODKEY, XK_3, change_workspace, {.i = 2} }
};

static Workspace workspaces[] = {
	{.layout = 0},
	{.layout = 1},
	{.layout = 2}
};

#endif
