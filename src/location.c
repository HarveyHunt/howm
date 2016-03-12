#include "howm.h"
#include "location.h"

/**
 * @file location.c
 *
 * @author Harvey Hunt
 *
 * @date 2016
 *
 * @brief howm
 */

/**
 * @brief Search all workspaces on all monitors for a window, populating
 * r_loc upon success.
 *
 * @param r_loc Will be populated upon finding the window.
 * @param win A valid XCB window that is used when searching all clients.
 * across all desktops.
 *
 * @return True upon finding the window, False otherwise.
 */
bool loc_win(location_t *r_loc, xcb_window_t win)
{
	monitor_t *m;
	workspace_t *ws;
	client_t *c;

	for (m = mon_head; m; m = m->next)
		for (ws = m->ws_head; ws; ws = ws->next)
			for (c = ws->head; c; c = c->next)
				if (win == c->win) {
					r_loc->mon = m;
					r_loc->ws = ws;
					r_loc->c = c;
					return true;
				}
	return false;
}

/**
 * @brief Search all workspaces on all monitors for a client, populating
 * loc upon success.
 *
 * @param r_loc Will be populated upon finding the client.
 * @param c A client that is used when searching all clients
 * across all desktops.
 *
 * @return True upon finding the client, False otherwise.
 */
inline bool loc_client(location_t *r_loc, client_t *c)
{
	if (c)
		return loc_win(r_loc, c->win);

	return false;
}
