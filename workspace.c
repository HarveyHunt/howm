#include "workspace.h"

/**
 * @brief Change to a different workspace and map the correct windows.
 *
 * @param arg arg->i indicates which workspace howm should change to.
 */
void change_ws(const Arg *arg)
{
	Client *c = wss[arg->i].head;

	if (arg->i > WORKSPACES || arg->i <= 0 || arg->i == cw)
		return;
	last_ws = cw;
	log_info("Changing from workspace <%d> to <%d>.", last_ws, arg->i);
	for (; c; c = c->next)
		xcb_map_window(dpy, c->win);
	for (c = wss[last_ws].head; c; c = c->next)
		xcb_unmap_window(dpy, c->win);
	cw = arg->i;
	update_focused_client(wss[cw].current);

	xcb_ewmh_set_current_desktop(ewmh, 0, cw - 1);
	xcb_ewmh_geometry_t workarea[] = { { 0, BAR_BOTTOM ? 0 : wss[cw].bar_height,
				screen_width, screen_height - wss[cw].bar_height } };
	xcb_ewmh_set_workarea(ewmh, 0, LENGTH(workarea), workarea);

	howm_info();
}

/**
 * @brief Focus the previous workspace.
 *
 * @param arg Unused.
 */
void focus_prev_ws(const Arg *arg)
{
	UNUSED(arg);

	log_info("Focusing previous workspace");
	change_ws(&(Arg){ .i = correct_ws(cw - 1) });
}

/**
 * @brief Focus the last focused workspace.
 *
 * @param arg Unused.
 */
void focus_last_ws(const Arg *arg)
{
	UNUSED(arg);

	log_info("Focusing last workspace");
	change_ws(&(Arg){ .i = last_ws });
}

/**
 * @brief Focus the next workspace.
 *
 * @param arg Unused.
 */
void focus_next_ws(const Arg *arg)
{
	UNUSED(arg);

	log_info("Focusing previous workspace");
	change_ws(&(Arg){ .i = correct_ws(cw + 1) });
}

/**
 * @brief Kills the given workspace.
 *
 * @param ws The workspace to be killed.
 */
void kill_ws(const int ws)
{
	log_info("Killing off workspace <%d>", ws);
	while (wss[ws].head)
		kill_client(ws, wss[ws].client_cnt == 1
				&& cw == ws);
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
	if (ws < 1)
		return ws + WORKSPACES;

	return ws;
}

