#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <xcb/xcb.h>
#include <xcb/xcb_ewmh.h>
#include <xcb/xcb_icccm.h>

#include "client.h"
#include "helper.h"
#include "howm.h"
#include "layout.h"
#include "scratchpad.h"
#include "workspace.h"
#include "xcb_help.h"

/**
 * @file client.c
 *
 * @author Harvey Hunt
 *
 * @date 2015
 *
 * @brief Operations that are to be performed on clients, such as moving them
 * around in the client list.
 */

static void move_down(client_t *c);

/**
 * @brief Search workspaces for a window, returning the client that it belongs
 * to.
 *
 * @param win A valid XCB window that is used when searching all clients across
 * all desktops.
 *
 * @return The found client.
 */
client_t *find_client_by_win(xcb_window_t win)
{
	bool found;
	workspace_t *ws;
	client_t *c = NULL;

	for (found = false, ws = mon->ws_head; ws != NULL && !found; ws = ws->next)
		for (c = ws->head; c && !(found = (win == c->win)); c = c->next)
			;
	return c;
}

/**
 * @brief Find the client before the given client.
 *
 * @param c The client which needs to have its previous found.
 *
 * @param w The workspace that the client is on.
 *
 * @return The previous client, so long as the given client isn't NULL and
 * there is more than one client. Else, NULL.
 */
client_t *prev_client(client_t *c, workspace_t *w)
{
	client_t *p = NULL;

	if (!c || !w->head || !w->head->next)
		return NULL;
	for (p = w->head; p->next && p->next != c; p = p->next)
		;
	return p;
}

/**
 * @brief Find the next client.
 *
 * Note: This function wraps around the end of the list of clients. If c is the
 * last item in the list of clients, then the head of the list is returned.
 *
 * @param c The client which needs to have its next found.
 *
 * @return The next client, if c is the last client in the list then this will
 * be head. If c is NULL or there is only one client in the client list, NULL
 * will be returned.
 */
client_t *next_client(client_t *c)
{
	if (!c || !mon->ws->head	|| !mon->ws->head->next)
		return NULL;
	if (c->next)
		return c->next;
	return mon->ws->head;
}

/**
 * @brief Sets c to the active window and gives it input focus. Sorts out
 * border colours as well.
 *
 * WARNING: Do NOT use this to focus a client on another workspace. Instead,
 * set wss[ws].current to the client that you want focused.
 *
 * @param c The client that is currently in focus.
 */
void update_focused_client(client_t *c)
{
	unsigned int all = 0, fullscreen = 0, float_trans = 0;

	if (!c)
		return;

	if (!mon->ws->head) {
		mon->ws->prev_foc = mon->ws->c = NULL;
		xcb_ewmh_set_active_window(ewmh, 0, XCB_NONE);
		return;
	} else if (c == mon->ws->prev_foc) {
		mon->ws->prev_foc = prev_client(mon->ws->c = mon->ws->prev_foc, mon->ws);
	} else if (c != mon->ws->c) {
		mon->ws->prev_foc = mon->ws->c;
		mon->ws->c = c;
	}

	log_info("Focusing client <%p>", c);
	for (c = mon->ws->head; c; c = c->next, ++all) {
		if (FFT(c)) {
			fullscreen++;
			if (!c->is_fullscreen)
				float_trans++;
		}
	}
	xcb_window_t windows[all];
	memset(windows, 0, sizeof(windows));

	windows[(mon->ws->c->is_floating || mon->ws->c->is_transient) ? 0 : float_trans] = mon->ws->c->win;
	c = mon->ws->head;
	for (fullscreen += !FFT(mon->ws->c) ? 1 : 0; c; c = c->next) {
		set_border_width(c->win, c->is_fullscreen ? 0 : conf.border_px);
		xcb_change_window_attributes(dpy, c->win, XCB_CW_BORDER_PIXEL,
					     (c == mon->ws->c ? &conf.border_focus :
					      c == mon->ws->prev_foc ? &conf.border_prev_focus
					      : &conf.border_unfocus));
		if (c != mon->ws->c)
			windows[c->is_fullscreen ? --fullscreen : FFT(c) ?
				--float_trans : --all] = c->win;
	}

	for (float_trans = 1; float_trans <= all; ++float_trans)
		elevate_window(windows[all - float_trans]);

	xcb_ewmh_set_active_window(ewmh, 0, mon->ws->c->win);

	xcb_set_input_focus(dpy, XCB_INPUT_FOCUS_POINTER_ROOT, mon->ws->c->win,
			    XCB_CURRENT_TIME);
	arrange_windows();
}

/**
 * @brief Count how many clients aren't Transient, Floating or Fullscreen.
 *
 * @return The amount of clients in the current workspace that aren't TFF.
 */
int get_non_tff_count(void)
{
	int n = 0;
	client_t *c = NULL;

	for (c = mon->ws->head; c; c = c->next)
		if (!FFT(c))
			n++;
	return n;
}

/**
 * @brief Returns the first client that isn't transient, floating or
 * fullscreen.
 *
 * @return The first client that isn't TFF. NULL if none.
 */
client_t *get_first_non_tff(void)
{
	client_t *c = NULL;

	for (c = mon->ws->head; c && FFT(c); c = c->next)
		;
	return c;
}

/**
 * @brief Remove a client from its workspace client list.
 *
 * @param c The client to be removed.
 *
 * @param refocus Whether the clients should be rearranged and focus be
 * updated.
 */
void remove_client(client_t *c, bool refocus)
{
	client_t **temp = NULL;
	workspace_t *w = mon->ws_head;

	for (; w != NULL ; w = w->next)
		for (temp = &w->head; *temp; temp = &(*temp)->next)
			if (*temp == c)
				goto found;
	return;

found:
	*temp = c->next;
	log_info("Removing client <%p>", c);
	if (c == w->prev_foc)
		w->prev_foc = prev_client(w->c, w);
	if (c == w->c || !w->head->next) {
		w->c = w->prev_foc ? w->prev_foc : w->head;
		if (refocus)
			update_focused_client(w->c);
	}
	free(c);
	c = NULL;
	w->client_cnt--;
}

/**
 * @brief Move a client down in its client list.
 *
 * @param c The client to be moved.
 */
static void move_down(client_t *c)
{
	client_t *prev = prev_client(c, mon->ws);
	client_t *n = (c->next) ? c->next : mon->ws->head;

	if (!c)
		return;
	if (!prev)
		return;
	if (mon->ws->head == c)
		mon->ws->head = n;
	else
		prev->next = c->next;
	c->next = (c->next) ? n->next : n;
	if (n->next == c->next)
		n->next = c;
	else
		mon->ws->head = c;
	log_info("Moved client <%p> on workspace <%d> down",
				c, workspace_to_index(mon->ws));
	arrange_windows();
}

/**
 * @brief Move a client up in its client list.
 *
 * @param c The client to be moved down.
 */
void move_up(client_t *c)
{
	client_t *p = prev_client(c, mon->ws);
	client_t *pp = NULL;

	if (!c)
		return;
	if (!p)
		return;
	if (p->next)
		for (pp = mon->ws->head; pp && pp->next != p; pp = pp->next)
			;
	if (pp)
		pp->next = c;
	else
		mon->ws->head = (mon->ws->head == c) ? c->next : c;
	p->next = (c->next == mon->ws->head) ? c : c->next;
	c->next = (c->next == mon->ws->head) ? NULL : p;
	log_info("Moved client <%p> on workspace <%d> down",
				c, workspace_to_index(mon->ws));
	arrange_windows();
}

/**
 * @brief brief Move focus onto the client next in the client list.
 *
 * @ingroup commands
 */
void focus_next_client(void)
{
	if (!mon->ws->c || !mon->ws->head->next)
		return;
	log_info("Focusing next client");
	update_focused_client(mon->ws->c->next ? mon->ws->c->next : mon->ws->head);
}

/**
 * @brief brief Move focus onto the client previous in the client list.
 *
 * @ingroup commands
 */
void focus_prev_client(void)
{
	if (!mon->ws->c || !mon->ws->head->next)
		return;
	log_info("Focusing previous client");
	mon->ws->prev_foc = mon->ws->c;
	update_focused_client(prev_client(mon->ws->prev_foc, mon->ws));
}

/**
 * @brief Kills the current client on the workspace w.
 *
 * @param w The workspace that the current client to be killed is on.
 *
 * @param arrange Whether the windows should be rearranged.
 */
void kill_client(workspace_t *w, bool arrange)
{
	xcb_icccm_get_wm_protocols_reply_t rep;
	unsigned int i;
	bool found = false;

	if (!w->c)
		return;

	if (xcb_icccm_get_wm_protocols_reply(dpy,
				xcb_icccm_get_wm_protocols(dpy,
					w->c->win,
					wm_atoms[WM_PROTOCOLS]), &rep, NULL)) {
		for (i = 0; i < rep.atoms_len; ++i)
			if (rep.atoms[i] == wm_atoms[WM_DELETE_WINDOW]) {
				delete_win(w->c->win);
				found = true;
				break;
			}
		xcb_icccm_get_wm_protocols_reply_wipe(&rep);
	}
	if (!found)
		xcb_kill_client(dpy, w->c->win);
	log_info("Killing Client <%p>", w->c);
	remove_client(w->c, arrange);
}

/**
 * @brief Moves a client either upwards or down.
 *
 * Moves a single client or multiple clients either up or
 * down. The op_move_* functions serves as simple wrappers to this.
 *
 * @param cnt How many clients to move.
 * @param up Whether to move the clients up or down. True is up.
 */
void move_client(int cnt, bool up)
{
	int cntcopy;
	client_t *c;

	if (up) {
		if (mon->ws->c == mon->ws->head)
			return;
		c = prev_client(mon->ws->c, mon->ws);
		/* TODO optimise this by inserting the client only once
			* and in the correct location.*/
		for (; cnt > 0; move_down(c), cnt--)
			;
	} else {
		if (mon->ws->c == prev_client(mon->ws->head, mon->ws))
			return;
		cntcopy = cnt;
		for (c = mon->ws->c; cntcopy > 0; c = next_client(c), cntcopy--)
			;
		for (; cnt > 0; move_up(c), cnt--)
			;
	}
}

/**
 * @brief Moves the current client down.
 *
 * @ingroup commands
 */
void move_current_down(void)
{
	move_down(mon->ws->c);
}

/**
 * @brief Moves the current client up.
 *
 * @ingroup commands
 */
void move_current_up(void)
{
	move_up(mon->ws->c);
}

/**
 * @brief Moves a client from one workspace to another.
 *
 * @param c The client to be moved.
 * @param ws The ws that the client should be moved to.
 * @param follow Should focus follow the client that has been moved?
 */
void client_to_ws(client_t *c, workspace_t *ws, bool follow)
{
	client_t *last;
	client_t *prev = prev_client(c, mon->ws);

	/* Performed for the current workspace. */
	if (!c || ws == mon->ws)
		return;
	/* Target workspace. */
	last = prev_client(ws->head, ws);
	if (!ws->head)
		ws->head = c;
	else if (last)
		last->next = c;
	else
		ws->head->next = c;
	ws->c = c;
	ws->client_cnt++;

	/* Current workspace. */
	if (c == mon->ws->head || !prev)
		mon->ws->head = c->next;
	else
		prev->next = c->next;
	mon->ws->c = prev;
	mon->ws->client_cnt--;

	c->next = NULL;
	xcb_unmap_window(dpy, c->win);

	log_info("Moved client <%p> from <%d> to <%d>", c,
			workspace_to_index(mon->ws),
			workspace_to_index(ws));
	if (follow) {
		ws->c = c;
		change_ws(ws);
	} else {
		update_focused_client(prev);
	}
}

/**
 * @brief Arrange the client's windows on the screen.
 *
 * This function takes some strain off of the layout handlers by passing the
 * client's dimensions to move_resize. This splits the layout handlers into
 * smaller, more understandable parts.
 */
void draw_clients(void)
{
	client_t *c = NULL;

	log_debug("Drawing clients");
	for (c = mon->ws->head; c; c = c->next)
		if (mon->ws->layout == ZOOM && conf.zoom_gap && !c->is_floating) {
			set_border_width(c->win, 0);
			move_resize(c->win, c->rect.x + c->gap, c->rect.y + c->gap,
					c->rect.width - (2 * c->gap), c->rect.height - (2 * c->gap));
		} else if (c->is_floating && !c->is_fullscreen) {
			set_border_width(c->win, conf.border_px);
			move_resize(c->win, c->rect.x, c->rect.y, c->rect.width, c->rect.height);
		} else if (c->is_fullscreen || mon->ws->layout == ZOOM) {
			set_border_width(c->win, 0);
			move_resize(c->win, c->rect.x, c->rect.y, c->rect.width, c->rect.height);
		} else {
			move_resize(c->win, c->rect.x + c->gap, c->rect.y + c->gap,
					c->rect.width - (2 * (c->gap + conf.border_px)),
					c->rect.height - (2 * (c->gap + conf.border_px)));
		}
}

/**
 * @brief Change the size and location of a client.
 *
 * @param c The client to be changed.
 * @param x The x coordinate of the client's window.
 * @param y The y coordinate of the client's window.
 * @param w The width of the client's window.
 * @param h The height of the client's window.
 */
void change_client_geom(client_t *c, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	log_debug("Changing geometry of client <%p> from {%d, %d, %d, %d} to {%d, %d, %d, %d}",
			c, c->rect.x, c->rect.y, c->rect.width, c->rect.height, x, y, w, h);
	c->rect = (xcb_rectangle_t) { x, y, w, h };
}

/**
 * @brief A helper function to change the size of a client's gaps.
 *
 * @param c The client who's gap size should be changed.
 * @param size The size by which the gap should be changed.
 */
void change_client_gaps(client_t *c, int size)
{
	if (c->is_fullscreen)
		return;
	if ((int)c->gap + size <= 0)
		c->gap = 0;
	else
		c->gap += size;

	uint32_t space = c->gap + conf.border_px;

	xcb_ewmh_set_frame_extents(ewmh, c->win, space, space, space, space);
	draw_clients();
}

/**
 * @brief Convert a window into a client.
 *
 * @param w A valid xcb window.
 *
 * @return A client that has already been inserted into the linked list of
 * clients.
 */
client_t *create_client(xcb_window_t w)
{
	client_t *c = (client_t *)calloc(1, sizeof(client_t));
	client_t *t = prev_client(mon->ws->head, mon->ws); /* Get the last element. */
	uint32_t vals[1] = { XCB_EVENT_MASK_PROPERTY_CHANGE |
				 (conf.focus_mouse ? XCB_EVENT_MASK_ENTER_WINDOW : 0)};

	if (!c) {
		log_err("Can't allocate memory for client.");
		exit(EXIT_FAILURE);
	}
	if (!mon->ws->head)
		mon->ws->head = c;
	else if (t)
		t->next = c;
	else
		mon->ws->head->next = c;
	c->win = w;
	c->gap = mon->ws->gap;
	xcb_change_window_attributes(dpy, c->win, XCB_CW_EVENT_MASK, vals);
	uint32_t space = c->gap + conf.border_px;

	xcb_ewmh_set_frame_extents(ewmh, c->win, space, space, space, space);
	log_info("Created client <%p>", c);
	mon->ws->client_cnt++;
	return c;
}

/**
 * @brief Set the fullscreen state of the client. Change its geometry and
 * border widths.
 *
 * @param c The client which should have its fullscreen state altered.
 * @param fscr The fullscreen state that the client should be changed to.
 */
void set_fullscreen(client_t *c, bool fscr)
{
	long data[] = {fscr ? ewmh->_NET_WM_STATE_FULLSCREEN : XCB_NONE };

	if (!c || fscr == c->is_fullscreen)
		return;

	c->is_fullscreen = fscr;
	log_info("Setting client <%p>'s fullscreen state to %d", c, fscr);
	xcb_change_property(dpy, XCB_PROP_MODE_REPLACE,
			c->win, ewmh->_NET_WM_STATE, XCB_ATOM_ATOM, 32,
			fscr, data);
	if (fscr) {
		set_border_width(c->win, 0);
		change_client_geom(c, 0, 0, screen_width, screen_height);
		draw_clients();
	} else {
		set_border_width(c->win, !mon->ws->head->next ? 0 : conf.border_px);
		arrange_windows();
		draw_clients();
	}
}

void set_urgent(client_t *c, bool urg)
{
	if (!c || urg == c->is_urgent)
		return;

	c->is_urgent = urg;
	xcb_change_window_attributes(dpy, c->win, XCB_CW_BORDER_PIXEL,
			urg ? &conf.border_urgent : c == mon->ws->c
			? &conf.border_focus : &conf.border_unfocus);
}

/**
 * @brief Teleport a floating client's window to a location on the screen.
 *
 * @param direction Which location to teleport the window to.
 *
 * @ingroup commands
 */
void teleport_client(const int direction)
{
	if (!mon->ws->c || !mon->ws->c->is_floating
			|| mon->ws->c->is_transient)
		return;

	/* A bit naughty, but it looks nicer- doesn't it?*/
	uint16_t g = mon->ws->c->gap;
	uint16_t w = mon->ws->c->rect.width;
	uint16_t h = mon->ws->c->rect.height;
	uint16_t bh = mon->ws->bar_height;

	switch (direction) {
	case TOP_LEFT:
		mon->ws->c->rect.x = g;
		mon->ws->c->rect.y = (conf.bar_bottom ? 0 : bh) + g;
		break;
	case TOP_CENTER:
		mon->ws->c->rect.x = (screen_width - w) / 2;
		mon->ws->c->rect.y = (conf.bar_bottom ? 0 : bh) + g;
		break;
	case TOP_RIGHT:
		mon->ws->c->rect.x = screen_width - w - g - (2 * conf.border_px);
		mon->ws->c->rect.y = (conf.bar_bottom ? 0 : bh) + g;
		break;
	case CENTER:
		mon->ws->c->rect.x = (screen_width - w) / 2;
		mon->ws->c->rect.y = (screen_height - bh - h) / 2;
		break;
	case BOTTOM_LEFT:
		mon->ws->c->rect.x = g;
		mon->ws->c->rect.y = (conf.bar_bottom ? screen_height - bh : screen_height) - h - g - (2 * conf.border_px);
		break;
	case BOTTOM_CENTER:
		mon->ws->c->rect.x = (screen_width / 2) - (w / 2);
		mon->ws->c->rect.y = (conf.bar_bottom ? screen_height - bh : screen_height) - h - g - (2 * conf.border_px);
		break;
	case BOTTOM_RIGHT:
		mon->ws->c->rect.x = screen_width - w - g - (2 * conf.border_px);
		mon->ws->c->rect.y = (conf.bar_bottom ? screen_height - bh : screen_height) - h - g - (2 * conf.border_px);
		break;
	};
	draw_clients();
}

/**
 * @brief Moves the current client to the workspace passed in.
 *
 * @param ws The target workspace.
 *
 * @ingroup commands
 */
void current_to_ws(workspace_t *ws)
{
	client_to_ws(mon->ws->c, ws, conf.follow_move);
}

/**
 * @brief Toggle a client between being in a floating or non-floating state.
 *
 * @ingroup commands
 */
void toggle_float(void)
{
	if (!mon->ws->c)
		return;
	log_info("Toggling floating state of client <%p>", mon->ws->c);
	mon->ws->c->is_floating = !mon->ws->c->is_floating;
	if (mon->ws->c->is_floating && conf.center_floating) {
		mon->ws->c->rect.x = (screen_width / 2) - (mon->ws->c->rect.width / 2);
		mon->ws->c->rect.y = (screen_height - mon->ws->bar_height - mon->ws->c->rect.height) / 2;
		log_info("Centering client <%p>", mon->ws->c);
	}
	arrange_windows();
}

/**
 * @brief Change the width of a floating client.
 *
 * Negative values will shift the right edge of the window to the left. The
 * inverse is true for positive values.
 *
 * @param dw The amount of pixels that the window's size should be changed by.
 *
 * @ingroup commands
 */
void resize_float_width(const int dw)
{
	if (!mon->ws->c || !mon->ws->c->is_floating || (int)mon->ws->c->rect.width + dw <= 0)
		return;
	log_info("Resizing width of client <%p> from %d by %d", mon->ws->c, mon->ws->c->rect.width, dw);
	mon->ws->c->rect.width += dw;
	draw_clients();
}

/**
 * @brief Change the height of a floating client.
 *
 * Negative values will shift the bottom edge of the window to the top. The
 * inverse is true for positive values.
 *
 * @param dh The amount of pixels that the window's size should be changed by.
 *
 * @ingroup commands
 */
void resize_float_height(const int dh)
{
	if (!mon->ws->c || !mon->ws->c->is_floating || (int)mon->ws->c->rect.height + dh <= 0)
		return;
	log_info("Resizing height of client <%p> from %d to %d", mon->ws->c, mon->ws->c->rect.height, dh);
	mon->ws->c->rect.height += dh;
	draw_clients();
}

/**
 * @brief Change a floating window's y coordinate.
 *
 * Negative values will move the window up. The inverse is true for positive
 * values.
 *
 * @param dy The amount of pixels that the window should be moved.
 *
 * @ingroup commands
 */
void move_float_y(const int dy)
{
	if (!mon->ws->c || !mon->ws->c->is_floating)
		return;
	log_info("Changing y of client <%p> from %d to %d", mon->ws->c, mon->ws->c->rect.y, dy);
	mon->ws->c->rect.y += dy;
	draw_clients();
}

/**
 * @brief Change a floating window's x coordinate.
 *
 * Negative values will move the window to the left. The inverse is true
 * for positive values.
 *
 * @param dx The amount of pixels that the window should be moved.
 *
 * @ingroup commands
 */
void move_float_x(const int dx)
{
	if (!mon->ws->c || !mon->ws->c->is_floating)
		return;
	log_info("Changing x of client <%p> from %d to %d", mon->ws->c, mon->ws->c->rect.x, dx);
	mon->ws->c->rect.x += dx;
	draw_clients();
}

/**
 * @brief Moves the current window to the master window, when in stack mode.
 *
 * @ingroup commands
 */
void make_master(void)
{
	if (!mon->ws->c || !mon->ws->head->next
			|| mon->ws->head == mon->ws->c
			|| !(mon->ws->layout == HSTACK
			|| mon->ws->layout == VSTACK))
		return;
	while (mon->ws->c != mon->ws->head)
		move_up(mon->ws->c);
	update_focused_client(mon->ws->head);
}

/**
 * @brief Toggle the fullscreen state of the current client.
 *
 * @ingroup commands
 */
void toggle_fullscreen(void)
{
	if (mon->ws->c != NULL)
		set_fullscreen(mon->ws->c, !mon->ws->c->is_fullscreen);
}

/**
 * @brief Focus a client that has an urgent hint.
 *
 * @ingroup commands
 */
void focus_urgent(void)
{
	client_t *c = NULL;
	workspace_t *w;

	for (w = mon->ws_head; w != NULL; w = w->next)
		for (c = w->head; c && !c->is_urgent; c = c->next)
			;
	if (c) {
		log_info("Focusing urgent client <%p> on workspace <%d>",
						c, workspace_to_index(w));
		change_ws(w);
		update_focused_client(c);
	}
}

/**
 * @brief Resize the master window of a stack for the current workspace.
 *
 * @param ds The amount to resize the master window by. Treated as a
 * percentage. e.g. ds = 5 will increase the master window's size by 5% of
 * it maximum.
 *
 * @ingroup commands
 */
void resize_master(const int ds)
{
	/* Resize master only when resizing is visible (i.e. in Stack layouts). */
	if (mon->ws->layout != HSTACK && mon->ws->layout != VSTACK)
		return;

	float change = ((float)ds) / 100;

	if (mon->ws->master_ratio + change >= 1
			|| mon->ws->master_ratio + change <= 0.1)
		return;
	log_info("Resizing master_ratio from <%.2f> to <%.2f>", mon->ws->master_ratio, mon->ws->master_ratio + change);
	mon->ws->master_ratio += change;
	arrange_windows();
}

/**
 * @brief Remove a list of clients from howm's delete register stack and paste
 * them after the currently focused window.
 *
 * @ingroup commands
 */
void paste(void)
{
	client_t *head = stack_pop(&del_reg);
	client_t *t, *c = head;

	if (!head) {
		log_warn("No clients on stack.");
		return;
	}

	if (!mon->ws->c) {
		mon->ws->head = head;
		mon->ws->c = head;
		while (c) {
			xcb_map_window(dpy, c->win);
			mon->ws->c = c;
			c = c->next;
			mon->ws->client_cnt++;
		}
	} else if (!mon->ws->c->next) {
		mon->ws->c->next = head;
		while (c) {
			xcb_map_window(dpy, c->win);
			mon->ws->c = c;
			c = c->next;
			mon->ws->client_cnt++;
		}
	} else {
		t = mon->ws->c->next;
		mon->ws->c->next = head;
		while (c) {
			xcb_map_window(dpy, c->win);
			mon->ws->client_cnt++;
			if (!c->next) {
				c->next = t;
				mon->ws->c = c;
				break;
			} else {
				mon->ws->c = c;
				c = c->next;
			}
		}
	}
	update_focused_client(mon->ws->c);
}

/**
 * @brief Toggle the space reserved for a status bar.
 *
 * @ingroup commands
 */
void toggle_bar(void)
{
	if (mon->ws->bar_height == 0 && conf.bar_height > 0) {
		mon->ws->bar_height = conf.bar_height;
		log_info("Toggled bar to shown");
	} else if (mon->ws->bar_height == conf.bar_height) {
		mon->ws->bar_height = 0;
		log_info("Toggled bar to hidden");
	} else {
		return;
	}
	xcb_ewmh_geometry_t workarea[] = { { 0, conf.bar_bottom ? 0 : mon->ws->bar_height,
				screen_width, screen_height - mon->ws->bar_height } };
	xcb_ewmh_set_workarea(ewmh, 0, LENGTH(workarea), workarea);
	arrange_windows();
}

