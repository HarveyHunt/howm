#include <xcb/xcb_ewmh.h>
#include <xcb/xproto.h>

#include "client.h"
#include "helper.h"
#include "howm.h"
#include "monitor.h"
#include "types.h"
#include "workspace.h"
#include "xcb_help.h"

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
 * @param m The monitor that the workspace to be killed is on.
 * @param ws The workspace to be killed.
 */
void kill_ws(monitor_t *m, workspace_t *ws)
{
	if (!ws || !ws->client_cnt)
		return;

	while (ws->head)
		kill_client(m, ws, m->ws->head);

	log_info("Killed off workspace <%d>", workspace_to_index(ws));
}

/**
 * @brief Correctly wrap a workspace number.
 *
 * This prevents workspace numbers from being greater than workspace_cnt or less
 * than 1.
 *
 * @param ws The value that needs to be corrected.
 *
 * @return A corrected workspace number.
 */
inline int correct_ws(unsigned int ws)
{
	if (ws > workspace_cnt)
		return ws - workspace_cnt;
	if (ws < 1)
		return ws + workspace_cnt;

	return ws;
}

/**
 * @brief Return a workspace after offsetting
 *
 * Use this to safely traverse the workspace list - instead of
 * doing:
 *
 *	ws->next->next
 *
 * do:
 *
 *	offset_ws(ws, 2)
 *
 * @param ws The workspace to be offset from.
 * @param offset The offset to apply
 *
 * @return The workspace at ws+offset.
 */
inline workspace_t *offset_ws(workspace_t *ws, int offset)
{
	workspace_t *ows = ws;
	bool pos = offset > 0 ? true : false;

	offset = abs(offset);
	for (; ows != NULL && offset > 0; ows = pos ? ws->next
					: ws->prev, offset--)
		;

	return ows;
}

/**
 * @brief Focus the previous workspace.
 *
 * @ingroup commands
 */
void focus_prev_ws(void)
{
	log_info("Focusing previous workspace");
	change_ws(mon->ws->prev);
}

/**
 * @brief Focus the last focused workspace.
 *
 * @ingroup commands
 */
void focus_last_ws(void)
{
	log_info("Focusing last workspace");
	change_ws(mon->last_ws);
}

/**
 * @brief Focus the next workspace.
 *
 * @ingroup commands
 */
void focus_next_ws(void)
{
	log_info("Focusing previous workspace");
	change_ws(mon->ws->next);
}

/**
 * @brief Change to a different workspace and map the correct windows.
 *
 * @param ws The workspace that howm should change to.
 *
 * @ingroup commands
 */
void change_ws(const workspace_t *ws)
{
	if (!ws)
		return;

	client_t *c = ws->head;

	mon->last_ws = mon->ws;
	log_debug("Changing from workspace <%d> to <%d>.", workspace_to_index(mon->last_ws),
							workspace_to_index(ws));

	for (; c; c = c->next)
		xcb_map_window(dpy, c->win);
	for (c = mon->last_ws->head; c; c = c->next)
		xcb_unmap_window(dpy, c->win);

	mon->ws = ws;

	update_focused_client(mon->ws->c);

	xcb_ewmh_set_current_desktop(ewmh, 0, workspace_to_index(ws));
	xcb_ewmh_geometry_t workarea[] = { { 0, conf.bar_bottom ? 0 : ws->bar_height,
				mon->rect.width, mon->rect.height - ws->bar_height } };
	xcb_ewmh_set_workarea(ewmh, 0, LENGTH(workarea), workarea);

	howm_info();
}

/**
 * @brief Find and return a workspace's index in the workspace list.
 *
 * @param ws The workspace to search for.
 *
 * @return The index of the workspace in the workspace list.
 */
uint32_t workspace_to_index(const workspace_t *ws)
{
	monitor_t *m;
	workspace_t *ows;
	uint32_t i = 0;

	for (m = mon_head; m != NULL; m = m->next)
		for (ows = m->ws_head; ows != NULL; ows = ows->next, i++)
			if (ws == ows)
				return i;
	return 0;
}

/**
 * @brief Convert a workspace's index in a workspace list to an index.
 *
 * @param m The monitor to search for the workspace on.
 * @param index The index to search for.
 *
 * @return The workspace stored at the index of the workspaces list.
 */
workspace_t *index_to_workspace(const monitor_t *m, uint32_t index)
{
	workspace_t *ws = m->ws_head;

	for (; index > 0 && ws != NULL; ws = ws->next, index--)
		;

	return ws;
}

/**
 * @brief Create a new workspace and update global state.
 *
 * @param m The monitor that the workspace should be added on.
 */
void add_ws(monitor_t *m)
{
	workspace_t *ws = calloc(1, sizeof(workspace_t));

	if (!ws) {
		log_err("Can't allocate memory for workspace");
		exit(EXIT_FAILURE);
	}

	ws->layout = WS_DEF_LAYOUT;
	ws->bar_height = conf.bar_height;
	ws->master_ratio = MASTER_RATIO;
	ws->gap = GAP;

	if (!m->ws) {
		m->ws = m->ws_tail = m->ws_head = ws;
	} else {
		m->ws_tail->next = ws;
		ws->prev = m->ws_tail;
		m->ws_tail = ws;
	}

	log_info("Added workspace <%d> to monitor <%d>",
			workspace_to_index(ws),
			monitor_to_index(m));

	m->workspace_cnt++;
	xcb_ewmh_set_number_of_desktops(ewmh, 0, m->workspace_cnt);
}

/**
 * @brief Remove a workspace and update the global state.
 *
 * @param m The monitor that the workspace is on.
 * @param ws The workspace to be removed.
 */
void remove_ws(monitor_t *m, workspace_t *ws)
{
	kill_ws(m, ws);
	if (m->ws == ws)
		change_ws(m->last_ws ? m->last_ws : m->ws_head);

	log_info("Removed workspace <%d>", workspace_to_index(ws));
	/* Sort out the workspaces list */
	if (ws->prev)
		ws->prev->next = ws->next;
	if (ws->next)
		ws->next->prev = ws->prev;

	/* Sort out the monitor list */
	if (m->ws_head == ws)
		m->ws_head = ws->next;
	if (m->ws_tail == ws)
		m->ws_tail = ws->prev;

	ws->head = ws->prev_foc = ws->c = NULL;
	ws->next = ws->prev = NULL;

	/* It seems reasonable to fall back to the first workspace */
	if (m->last_ws == ws)
		m->last_ws = m->ws_head;

	m->workspace_cnt--;
	ewmh_set_current_workspace();
	xcb_ewmh_set_number_of_desktops(ewmh, 0, m->workspace_cnt);

	free(ws);
}
