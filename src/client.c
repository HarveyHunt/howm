#include <stdbool.h>
#include <stdlib.h>
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
	unsigned int w = 1;
	client_t *c = NULL;

	for (found = false; w <= WORKSPACES && !found; w++)
		for (c = wss[w].head; c && !(found = (win == c->win)); c = c->next)
			;
	return c;
}

/**
 * @brief Find the client before the given client.
 *
 * @param c The client which needs to have its previous found.
 *
 * @param ws The workspace that the client is on.
 *
 * @return The previous client, so long as the given client isn't NULL and
 * there is more than one client. Else, NULL.
 */
client_t *prev_client(client_t *c, int ws)
{
	client_t *p = NULL;

	if (!c || !wss[ws].head || !wss[ws].head->next)
		return NULL;
	for (p = wss[ws].head; p->next && p->next != c; p = p->next)
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
	if (!c || !wss[cw].head	|| !wss[cw].head->next)
		return NULL;
	if (c->next)
		return c->next;
	return wss[cw].head;
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

	if (!wss[cw].head) {
		wss[cw].prev_foc = wss[cw].current = NULL;
		xcb_ewmh_set_active_window(ewmh, 0, XCB_NONE);
		return;
	} else if (c == wss[cw].prev_foc) {
		wss[cw].prev_foc = prev_client(wss[cw].current = wss[cw].prev_foc, cw);
	} else if (c != wss[cw].current) {
		wss[cw].prev_foc = wss[cw].current;
		wss[cw].current = c;
	}

	log_info("Focusing client <%p>", c);
	for (c = wss[cw].head; c; c = c->next, ++all) {
		if (FFT(c)) {
			fullscreen++;
			if (!c->is_fullscreen)
				float_trans++;
		}
	}
	xcb_window_t windows[all];

	windows[(wss[cw].current->is_floating || wss[cw].current->is_transient) ? 0 : float_trans] = wss[cw].current->win;
	c = wss[cw].head;
	for (fullscreen += !FFT(wss[cw].current) ? 1 : 0; c; c = c->next) {
		set_border_width(c->win, c->is_fullscreen ? 0 : conf.border_px);
		xcb_change_window_attributes(dpy, c->win, XCB_CW_BORDER_PIXEL,
					     (c == wss[cw].current ? &conf.border_focus :
					      c == wss[cw].prev_foc ? &conf.border_prev_focus
					      : &conf.border_unfocus));
		if (c != wss[cw].current)
			windows[c->is_fullscreen ? --fullscreen : FFT(c) ?
				--float_trans : --all] = c->win;
	}

	for (float_trans = 1; float_trans <= all; ++float_trans)
		elevate_window(windows[all - float_trans]);

	xcb_ewmh_set_active_window(ewmh, 0, wss[cw].current->win);

	xcb_set_input_focus(dpy, XCB_INPUT_FOCUS_POINTER_ROOT, wss[cw].current->win,
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

	for (c = wss[cw].head; c; c = c->next)
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

	for (c = wss[cw].head; c && FFT(c); c = c->next)
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
	unsigned int w = 1;

	for (; w <= WORKSPACES; w++)
		for (temp = &wss[w].head; *temp; temp = &(*temp)->next)
			if (*temp == c)
				goto found;
	return;

found:
	*temp = c->next;
	log_info("Removing client <%p>", c);
	if (c == wss[w].prev_foc)
		wss[w].prev_foc = prev_client(wss[w].current, w);
	if (c == wss[w].current || !wss[w].head->next) {
		wss[w].current = wss[w].prev_foc ? wss[w].prev_foc : wss[w].head;
		if (refocus)
			update_focused_client(wss[w].current);
	}
	free(c);
	c = NULL;
	wss[w].client_cnt--;
}

/**
 * @brief Move a client down in its client list.
 *
 * @param c The client to be moved.
 */
static void move_down(client_t *c)
{
	client_t *prev = prev_client(c, cw);
	client_t *n = (c->next) ? c->next : wss[cw].head;

	if (!c)
		return;
	if (!prev)
		return;
	if (wss[cw].head == c)
		wss[cw].head = n;
	else
		prev->next = c->next;
	c->next = (c->next) ? n->next : n;
	if (n->next == c->next)
		n->next = c;
	else
		wss[cw].head = c;
	log_info("Moved client <%p> on workspace <%d> down", c, cw);
	arrange_windows();
}

/**
 * @brief Move a client up in its client list.
 *
 * @param c The client to be moved down.
 */
void move_up(client_t *c)
{
	client_t *p = prev_client(c, cw);
	client_t *pp = NULL;

	if (!c)
		return;
	if (!p)
		return;
	if (p->next)
		for (pp = wss[cw].head; pp && pp->next != p; pp = pp->next)
			;
	if (pp)
		pp->next = c;
	else
		wss[cw].head = (wss[cw].head == c) ? c->next : c;
	p->next = (c->next == wss[cw].head) ? c : c->next;
	c->next = (c->next == wss[cw].head) ? NULL : p;
	log_info("Moved client <%p> on workspace <%d> down", c, cw);
	arrange_windows();
}

/**
 * @brief brief Move focus onto the client next in the client list.
 *
 * @ingroup commands
 */
void focus_next_client(void)
{
	if (!wss[cw].current || !wss[cw].head->next)
		return;
	log_info("Focusing next client");
	update_focused_client(wss[cw].current->next ? wss[cw].current->next : wss[cw].head);
}

/**
 * @brief brief Move focus onto the client previous in the client list.
 *
 * @ingroup commands
 */
void focus_prev_client(void)
{
	if (!wss[cw].current || !wss[cw].head->next)
		return;
	log_info("Focusing previous client");
	wss[cw].prev_foc = wss[cw].current;
	update_focused_client(prev_client(wss[cw].prev_foc, cw));
}

/**
 * @brief Kills the current client on the workspace ws.
 *
 * @param ws The workspace that the current client to be killed is on.
 *
 * @param arrange Whether the windows should be rearranged.
 */
void kill_client(const int ws, bool arrange)
{
	xcb_icccm_get_wm_protocols_reply_t rep;
	unsigned int i;
	bool found = false;

	if (!wss[ws].current)
		return;

	if (xcb_icccm_get_wm_protocols_reply(dpy,
				xcb_icccm_get_wm_protocols(dpy,
					wss[ws].current->win,
					wm_atoms[WM_PROTOCOLS]), &rep, NULL)) {
		for (i = 0; i < rep.atoms_len; ++i)
			if (rep.atoms[i] == wm_atoms[WM_DELETE_WINDOW]) {
				delete_win(wss[ws].current->win);
				found = true;
				break;
			}
		xcb_icccm_get_wm_protocols_reply_wipe(&rep);
	}
	if (!found)
		xcb_kill_client(dpy, wss[ws].current->win);
	log_info("Killing Client <%p>", wss[ws].current);
	remove_client(wss[ws].current, arrange);
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
		if (wss[cw].current == wss[cw].head)
			return;
		c = prev_client(wss[cw].current, cw);
		/* TODO optimise this by inserting the client only once
			* and in the correct location.*/
		for (; cnt > 0; move_down(c), cnt--)
			;
	} else {
		if (wss[cw].current == prev_client(wss[cw].head, cw))
			return;
		cntcopy = cnt;
		for (c = wss[cw].current; cntcopy > 0; c = next_client(c), cntcopy--)
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
	move_down(wss[cw].current);
}

/**
 * @brief Moves the current client up.
 *
 * @ingroup commands
 */
void move_current_up(void)
{
	move_up(wss[cw].current);
}

/**
 * @brief Moves a client from one workspace to another.
 *
 * @param c The client to be moved.
 * @param ws The ws that the client should be moved to.
 * @param follow Should focus follow the client that has been moved?
 */
void client_to_ws(client_t *c, const int ws, bool follow)
{
	client_t *last;
	client_t *prev = prev_client(c, cw);

	/* Performed for the current workspace. */
	if (!c || ws == cw)
		return;
	/* Target workspace. */
	last = prev_client(wss[ws].head, ws);
	if (!wss[ws].head)
		wss[ws].head = c;
	else if (last)
		last->next = c;
	else
		wss[ws].head->next = c;
	wss[ws].current = c;
	wss[ws].client_cnt++;

	/* Current workspace. */
	if (c == wss[cw].head || !prev)
		wss[cw].head = c->next;
	else
		prev->next = c->next;
	wss[cw].current = prev;
	wss[cw].client_cnt--;

	c->next = NULL;
	xcb_unmap_window(dpy, c->win);

	log_info("Moved client <%p> from <%d> to <%d>", c, cw, ws);
	if (follow) {
		wss[ws].current = c;
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
	for (c = wss[cw].head; c; c = c->next)
		if (wss[cw].layout == ZOOM && conf.zoom_gap && !c->is_floating) {
			set_border_width(c->win, 0);
			move_resize(c->win, c->rect.x + c->gap, c->rect.y + c->gap,
					c->rect.width - (2 * c->gap), c->rect.height - (2 * c->gap));
		} else if (c->is_floating && !c->is_fullscreen) {
			set_border_width(c->win, conf.border_px);
			move_resize(c->win, c->rect.x, c->rect.y, c->rect.width, c->rect.height);
		} else if (c->is_fullscreen || wss[cw].layout == ZOOM) {
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
	client_t *t = prev_client(wss[cw].head, cw); /* Get the last element. */
	uint32_t vals[1] = { XCB_EVENT_MASK_PROPERTY_CHANGE |
				 (conf.focus_mouse ? XCB_EVENT_MASK_ENTER_WINDOW : 0)};

	if (!c) {
		log_err("Can't allocate memory for client.");
		exit(EXIT_FAILURE);
	}
	if (!wss[cw].head)
		wss[cw].head = c;
	else if (t)
		t->next = c;
	else
		wss[cw].head->next = c;
	c->win = w;
	c->gap = wss[cw].gap;
	xcb_change_window_attributes(dpy, c->win, XCB_CW_EVENT_MASK, vals);
	uint32_t space = c->gap + conf.border_px;

	xcb_ewmh_set_frame_extents(ewmh, c->win, space, space, space, space);
	log_info("Created client <%p>", c);
	wss[cw].client_cnt++;
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
		set_border_width(c->win, !wss[cw].head->next ? 0 : conf.border_px);
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
			urg ? &conf.border_urgent : c == wss[cw].current
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
	if (!wss[cw].current || !wss[cw].current->is_floating
			|| wss[cw].current->is_transient)
		return;

	/* A bit naughty, but it looks nicer- doesn't it?*/
	uint16_t g = wss[cw].current->gap;
	uint16_t w = wss[cw].current->rect.width;
	uint16_t h = wss[cw].current->rect.height;
	uint16_t bh = wss[cw].bar_height;

	switch (direction) {
	case TOP_LEFT:
		wss[cw].current->rect.x = g;
		wss[cw].current->rect.y = (conf.bar_bottom ? 0 : bh) + g;
		break;
	case TOP_CENTER:
		wss[cw].current->rect.x = (screen_width - w) / 2;
		wss[cw].current->rect.y = (conf.bar_bottom ? 0 : bh) + g;
		break;
	case TOP_RIGHT:
		wss[cw].current->rect.x = screen_width - w - g - (2 * conf.border_px);
		wss[cw].current->rect.y = (conf.bar_bottom ? 0 : bh) + g;
		break;
	case CENTER:
		wss[cw].current->rect.x = (screen_width - w) / 2;
		wss[cw].current->rect.y = (screen_height - bh - h) / 2;
		break;
	case BOTTOM_LEFT:
		wss[cw].current->rect.x = g;
		wss[cw].current->rect.y = (conf.bar_bottom ? screen_height - bh : screen_height) - h - g - (2 * conf.border_px);
		break;
	case BOTTOM_CENTER:
		wss[cw].current->rect.x = (screen_width / 2) - (w / 2);
		wss[cw].current->rect.y = (conf.bar_bottom ? screen_height - bh : screen_height) - h - g - (2 * conf.border_px);
		break;
	case BOTTOM_RIGHT:
		wss[cw].current->rect.x = screen_width - w - g - (2 * conf.border_px);
		wss[cw].current->rect.y = (conf.bar_bottom ? screen_height - bh : screen_height) - h - g - (2 * conf.border_px);
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
void current_to_ws(const int ws)
{
	client_to_ws(wss[cw].current, ws, conf.follow_move);
}

/**
 * @brief Toggle a client between being in a floating or non-floating state.
 *
 * @ingroup commands
 */
void toggle_float(void)
{
	if (!wss[cw].current)
		return;
	log_info("Toggling floating state of client <%p>", wss[cw].current);
	wss[cw].current->is_floating = !wss[cw].current->is_floating;
	if (wss[cw].current->is_floating && conf.center_floating) {
		wss[cw].current->rect.x = (screen_width / 2) - (wss[cw].current->rect.width / 2);
		wss[cw].current->rect.y = (screen_height - wss[cw].bar_height - wss[cw].current->rect.height) / 2;
		log_info("Centering client <%p>", wss[cw].current);
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
	if (!wss[cw].current || !wss[cw].current->is_floating || (int)wss[cw].current->rect.width + dw <= 0)
		return;
	log_info("Resizing width of client <%p> from %d by %d", wss[cw].current, wss[cw].current->rect.width, dw);
	wss[cw].current->rect.width += dw;
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
	if (!wss[cw].current || !wss[cw].current->is_floating || (int)wss[cw].current->rect.height + dh <= 0)
		return;
	log_info("Resizing height of client <%p> from %d to %d", wss[cw].current, wss[cw].current->rect.height, dh);
	wss[cw].current->rect.height += dh;
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
	if (!wss[cw].current || !wss[cw].current->is_floating)
		return;
	log_info("Changing y of client <%p> from %d to %d", wss[cw].current, wss[cw].current->rect.y, dy);
	wss[cw].current->rect.y += dy;
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
	if (!wss[cw].current || !wss[cw].current->is_floating)
		return;
	log_info("Changing x of client <%p> from %d to %d", wss[cw].current, wss[cw].current->rect.x, dx);
	wss[cw].current->rect.x += dx;
	draw_clients();
}

/**
 * @brief Moves the current window to the master window, when in stack mode.
 *
 * @ingroup commands
 */
void make_master(void)
{
	if (!wss[cw].current || !wss[cw].head->next
			|| wss[cw].head == wss[cw].current
			|| !(wss[cw].layout == HSTACK
			|| wss[cw].layout == VSTACK))
		return;
	while (wss[cw].current != wss[cw].head)
		move_up(wss[cw].current);
	update_focused_client(wss[cw].head);
}

/**
 * @brief Toggle the fullscreen state of the current client.
 *
 * @ingroup commands
 */
void toggle_fullscreen(void)
{
	if (wss[cw].current != NULL)
		set_fullscreen(wss[cw].current, !wss[cw].current->is_fullscreen);
}

/**
 * @brief Focus a client that has an urgent hint.
 *
 * @ingroup commands
 */
void focus_urgent(void)
{
	client_t *c;
	unsigned int w;

	for (w = 1; w <= WORKSPACES; w++)
		for (c = wss[w].head; c && !c->is_urgent; c = c->next)
			;
	if (c) {
		log_info("Focusing urgent client <%p> on workspace <%d>", c, w);
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
	if (wss[cw].layout != HSTACK && wss[cw].layout != VSTACK)
		return;

	float change = ((float)ds) / 100;

	if (wss[cw].master_ratio + change >= 1
			|| wss[cw].master_ratio + change <= 0.1)
		return;
	log_info("Resizing master_ratio from <%.2f> to <%.2f>", wss[cw].master_ratio, wss[cw].master_ratio + change);
	wss[cw].master_ratio += change;
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

	if (!wss[cw].current) {
		wss[cw].head = head;
		wss[cw].current = head;
		while (c) {
			xcb_map_window(dpy, c->win);
			wss[cw].current = c;
			c = c->next;
			wss[cw].client_cnt++;
		}
	} else if (!wss[cw].current->next) {
		wss[cw].current->next = head;
		while (c) {
			xcb_map_window(dpy, c->win);
			wss[cw].current = c;
			c = c->next;
			wss[cw].client_cnt++;
		}
	} else {
		t = wss[cw].current->next;
		wss[cw].current->next = head;
		while (c) {
			xcb_map_window(dpy, c->win);
			wss[cw].client_cnt++;
			if (!c->next) {
				c->next = t;
				wss[cw].current = c;
				break;
			} else {
				wss[cw].current = c;
				c = c->next;
			}
		}
	}
	update_focused_client(wss[cw].current);
}

/**
 * @brief Toggle the space reserved for a status bar.
 *
 * @ingroup commands
 */
void toggle_bar(void)
{
	if (wss[cw].bar_height == 0 && conf.bar_height > 0) {
		wss[cw].bar_height = conf.bar_height;
		log_info("Toggled bar to shown");
	} else if (wss[cw].bar_height == conf.bar_height) {
		wss[cw].bar_height = 0;
		log_info("Toggled bar to hidden");
	} else {
		return;
	}
	xcb_ewmh_geometry_t workarea[] = { { 0, conf.bar_bottom ? 0 : wss[cw].bar_height,
				screen_width, screen_height - wss[cw].bar_height } };
	xcb_ewmh_set_workarea(ewmh, 0, LENGTH(workarea), workarea);
	arrange_windows();
}

