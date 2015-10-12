#include <xcb/xcb_ewmh.h>
#include <xcb/xproto.h>

#include "client.h"
#include "helper.h"
#include "howm.h"
#include "types.h"
#include "workspace.h"

/**
 * @file workspace.c
 *
 * @author Harvey Hunt
 *
 * @date 2015
 *
 * @brief Helper functions for dealing with an entire workspace or being able
 * to correctly calculate a workspace index.
 */

/**
 * @brief Kills the given workspace.
 *
 * @param ws The workspace to be killed.
 */
void kill_ws(const int ws)
{
	if (!wss[ws].client_cnt)
		return;

	while (wss[ws].head)
		kill_client(ws, wss[ws].client_cnt == 1
				&& cw == ws);

	log_info("Killed off workspace <%d>", ws);
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
inline int correct_ws(unsigned int ws)
{
	if (ws > WORKSPACES)
		return ws - WORKSPACES;
	if (ws < 1)
		return ws + WORKSPACES;

	return ws;
}

/**
 * @brief Focus the previous workspace.
 *
 * @ingroup commands
 */
void focus_prev_ws(void)
{
	log_info("Focusing previous workspace");
	change_ws(correct_ws(cw - 1));
}

/**
 * @brief Focus the last focused workspace.
 *
 * @ingroup commands
 */
void focus_last_ws(void)
{
	log_info("Focusing last workspace");
	change_ws(last_ws);
}

/**
 * @brief Focus the next workspace.
 *
 * @ingroup commands
 */
void focus_next_ws(void)
{
	log_info("Focusing previous workspace");
	change_ws(correct_ws(cw + 1));
}

/**
 * @brief Change to a different workspace and map the correct windows.
 *
 * @param ws Indicates which workspace howm should change to.
 *
 * @ingroup commands
 */
void change_ws(const int ws)
{
	Client *c = wss[ws].head;

	if ((unsigned int)ws > WORKSPACES || ws <= 0 || ws == cw)
		return;
	last_ws = cw;
	log_info("Changing from workspace <%d> to <%d>.", last_ws, ws);
	for (; c; c = c->next)
		xcb_map_window(dpy, c->win);
	for (c = wss[last_ws].head; c; c = c->next)
		xcb_unmap_window(dpy, c->win);
	cw = ws;
	update_focused_client(wss[cw].current);

	xcb_ewmh_set_current_desktop(ewmh, 0, cw - 1);
	xcb_ewmh_geometry_t workarea[] = { { 0, conf.bar_bottom ? 0 : wss[cw].bar_height,
				screen_width, screen_height - wss[cw].bar_height } };
	xcb_ewmh_set_workarea(ewmh, 0, LENGTH(workarea), workarea);

	howm_info();
}
