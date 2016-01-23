#include <stdlib.h>
#include <string.h>
#include <xcb/randr.h>
#include <xcb/xcb.h>
#include <xcb/xcb_ewmh.h>

#include "client.h"
#include "helper.h"
#include "howm.h"
#include "location.h"
#include "workspace.h"
#include "xcb_help.h"

/**
 * @file xcb_help.c
 *
 * @author Harvey Hunt
 *
 * @date 2015
 *
 * @brief The portion of howm that interacts with the X server. Perhaps this
 * could be conditionally included if we decide to use wayland as well.
 */

/**
 * @brief Try to detect if another WM exists.
 *
 * If another WM exists (this can be seen by whether it has registered itself
 * with the X11 server) then howm will exit.
 */
void check_other_wm(void)
{
	xcb_generic_error_t *e;
	uint32_t values[1] = { XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT |
			       XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY |
			       XCB_EVENT_MASK_BUTTON_PRESS |
			       XCB_EVENT_MASK_ENTER_WINDOW |
			       XCB_EVENT_MASK_PROPERTY_CHANGE
			     };

	e = xcb_request_check(dpy, xcb_change_window_attributes_checked(dpy,
			      screen->root, XCB_CW_EVENT_MASK, values));
	if (e != NULL) {
		xcb_disconnect(dpy);
		log_err("Couldn't register as WM. Perhaps another WM is running? XCB returned error_code: %d", e->error_code);
		exit(EXIT_FAILURE);
	}
	free(e);
}

/**
 * @brief Change the dimensions and location of a window (win).
 *
 * @param win The window upon which the operations should be performed.
 * @param x The new x location of the top left corner.
 * @param y The new y location of the top left corner.
 * @param w The new width of the window.
 * @param h The new height of the window.
 */
void move_resize(xcb_window_t win,
		 uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	uint32_t position[] = { x, y, w, h };

	xcb_configure_window(dpy, win, MOVE_RESIZE_MASK, position);
}

/**
 * @brief Make a client listen for button press events.
 *
 * @param c The client that needs to listen for button presses.
 */
void grab_buttons(client_t *c)
{
	xcb_ungrab_button(dpy, XCB_BUTTON_INDEX_ANY, c->win, XCB_GRAB_ANY);
	xcb_grab_button(dpy, 1, c->win, XCB_EVENT_MASK_BUTTON_PRESS,
			XCB_GRAB_MODE_SYNC, XCB_GRAB_MODE_ASYNC,
			XCB_WINDOW_NONE, XCB_CURSOR_NONE,
			XCB_BUTTON_INDEX_ANY, XCB_BUTTON_MASK_ANY);
}

/**
 * @brief Sets the width of the borders around a window (win).
 *
 * @param win The window that will have its border width changed.
 * @param w The new width of the window's border.
 */
void set_border_width(xcb_window_t win, uint16_t w)
{
	uint32_t width[1] = { w };

	xcb_configure_window(dpy, win, XCB_CONFIG_WINDOW_BORDER_WIDTH, width);
}

/**
 * @brief Move a window to the front of all the other windows.
 *
 * @param win The window to be moved.
 */
void elevate_window(xcb_window_t win)
{
	uint32_t stack_mode[1] = { XCB_STACK_MODE_ABOVE };

	log_info("Moving window <0x%x> to the front", win);
	xcb_configure_window(dpy, win, XCB_CONFIG_WINDOW_STACK_MODE, stack_mode);
}

/**
 * @brief Request all of the atoms that howm supports.
 *
 * @param names The names of the atoms to be fetched.
 * @param atoms Where the returned atoms will be stored.
 */
void get_atoms(const char **names, xcb_atom_t *atoms)
{
	xcb_intern_atom_reply_t *reply;
	unsigned int i = 0;
	xcb_intern_atom_cookie_t cookies[LENGTH(*names)];

	for (i = 0; i < LENGTH(atoms); i++) {
		cookies[i] = xcb_intern_atom(dpy, 0, strlen(names[i]), names[i]);
		log_debug("Requesting atom %s", names[i]);
	}
	for (i = 0; i < LENGTH(atoms); i++) {
		reply = xcb_intern_atom_reply(dpy, cookies[i], NULL);
		if (reply) {
			atoms[i] = reply->atom;
			log_debug("Got reply for atom %s", names[i]);
			free(reply);
		} else {
			log_warn("The atom %s has not been registered by howm.", names[i]);
		}
	}
}

/**
 * @brief Focus the given window, so long as it isn't already focused.
 *
 * @param win A window that belongs to a client being managed by howm.
 */
void focus_window(xcb_window_t win)
{
	location_t loc;

	if (loc_win(&loc, win) && loc.c != mon->ws->c)
		update_focused_client(loc.c);
	else
		/* We don't want warnings for clicking the root window... */
		if (win != screen->root)
			log_warn("No client owns the window <0x%x>", win);
}

/**
 * @brief Ask XCB to delete a window.
 *
 * @param win The window to be deleted.
 */
void delete_win(xcb_window_t win)
{
	xcb_client_message_event_t ev;

	log_info("Sending WM_DELETE_WINDOW to window <0x%x>", win);
	ev.response_type = XCB_CLIENT_MESSAGE;
	ev.sequence = 0;
	ev.format = 32;
	ev.window = win;
	ev.type = wm_atoms[WM_PROTOCOLS];
	ev.data.data32[0] = wm_atoms[WM_DELETE_WINDOW];
	ev.data.data32[1] = XCB_CURRENT_TIME;
	xcb_send_event(dpy, 0, win, XCB_EVENT_MASK_NO_EVENT, (char *)&ev);
}

/**
 * @brief Handle client messages that are related to WM_STATE.
 *
 * TODO: Add more WM_STATE hints.
 *
 * @param c The client that is to have its WM_STATE modified.
 * @param a The atom representing which WM_STATE hint should be modified.
 * @param action Whether to remove, add or toggle the WM_STATE hint.
 */
void ewmh_process_wm_state(client_t *c, xcb_atom_t a, int action)
{
	if (a == ewmh->_NET_WM_STATE_FULLSCREEN) {
		if (action == XCB_EWMH_WM_STATE_REMOVE)
			set_fullscreen(c, false);
		else if (action == XCB_EWMH_WM_STATE_ADD)
			set_fullscreen(c, true);
		else if (action == XCB_EWMH_WM_STATE_TOGGLE)
			set_fullscreen(c, !c->is_fullscreen);
	} else if (a == ewmh->_NET_WM_STATE_DEMANDS_ATTENTION) {
		if (action == XCB_EWMH_WM_STATE_REMOVE)
			set_urgent(c, false);
		else if (action == XCB_EWMH_WM_STATE_ADD)
			set_urgent(c, true);
		else if (action == XCB_EWMH_WM_STATE_TOGGLE)
			set_urgent(c, !c->is_urgent);
	} else {
		log_warn("Unhandled wm state <%d> with action <%d>.", a, action);
	}
}

/**
* @brief Create the EWMH connection, request all of the atoms and set some
* sensible defaults for them.
*/
void setup_ewmh(void)
{
	ewmh = calloc(1, sizeof(xcb_ewmh_connection_t));
	if (!ewmh) {
		log_err("Unable to create ewmh connection\n");
		exit(EXIT_FAILURE);
	}
	if (xcb_ewmh_init_atoms_replies(ewmh, xcb_ewmh_init_atoms(dpy, ewmh), NULL) == 0)
		log_err("Couldn't initialise ewmh atoms");
	xcb_atom_t ewmh_net_atoms[] = { ewmh->_NET_SUPPORTED,
					ewmh->_NET_SUPPORTING_WM_CHECK,
					ewmh->_NET_DESKTOP_VIEWPORT,
					ewmh->_NET_WM_NAME,
					ewmh->_NET_WM_STATE,
					ewmh->_NET_CLOSE_WINDOW,
					ewmh->_NET_WM_STATE_FULLSCREEN,
					ewmh->_NET_CURRENT_DESKTOP,
					ewmh->_NET_NUMBER_OF_DESKTOPS,
					ewmh->_NET_DESKTOP_GEOMETRY,
					ewmh->_NET_WORKAREA,
					ewmh->_NET_ACTIVE_WINDOW };
	xcb_ewmh_set_supported(ewmh, 0, LENGTH(ewmh_net_atoms), ewmh_net_atoms);
	xcb_ewmh_set_supporting_wm_check(ewmh, 0, screen->root);
	xcb_ewmh_set_wm_name(ewmh, 0, strlen("howm"), "howm");
}

void setup_ewmh_geom(void)
{
	xcb_ewmh_coordinates_t viewport[] = { {0, 0} };
	xcb_ewmh_geometry_t workarea[] = { {0, conf.bar_bottom ? 0
						: mon->ws->bar_height, mon->rect.width,
						mon->rect.height - mon->ws->bar_height} };

	xcb_ewmh_set_desktop_viewport(ewmh, 0, LENGTH(viewport), viewport);
	xcb_ewmh_set_workarea(ewmh, 0, LENGTH(workarea), workarea);
	xcb_ewmh_set_desktop_geometry(ewmh, 0, mon->rect.width, mon->rect.height);
}

void ewmh_set_current_workspace(void)
{
	xcb_ewmh_set_current_desktop(ewmh, 0, workspace_to_index(mon->ws));
}

xcb_randr_output_t *randr_get_outputs(unsigned int *nr_outputs)
{
	xcb_randr_get_screen_resources_reply_t *sresr;
	xcb_randr_get_screen_resources_cookie_t sresc;
	xcb_randr_output_t *outputs;
	const xcb_query_extension_reply_t *qer = xcb_get_extension_data(dpy,
								&xcb_randr_id);

	if (!qer || !qer->present)
		return false;

	sresc = xcb_randr_get_screen_resources(dpy, screen->root);
	sresr = xcb_randr_get_screen_resources_reply(dpy, sresc, NULL);
	*nr_outputs = xcb_randr_get_screen_resources_outputs_length(sresr);

	if (!sresr || *nr_outputs < 1)
		goto free_sresr;

	outputs = xcb_randr_get_screen_resources_outputs(sresr);

	return outputs;

free_sresr:
	free(sresr);
	return NULL;
}

xcb_rectangle_t output_reply_to_rect(xcb_randr_get_output_info_reply_t *output)
{
	xcb_randr_get_crtc_info_cookie_t cinfoc;
	xcb_randr_get_crtc_info_reply_t *cinfor;
	xcb_rectangle_t rect = {-1, -1, -1, -1};

	if (!output || output->crtc == XCB_NONE)
		return rect;

	cinfoc = xcb_randr_get_crtc_info(dpy, output->crtc, XCB_CURRENT_TIME);
	cinfor = xcb_randr_get_crtc_info_reply(dpy, cinfoc, NULL);

	if (cinfor)
		rect = (xcb_rectangle_t){cinfor->x, cinfor->y,
				cinfor->width, cinfor->height};

	free(cinfor);
	return rect;
}

xcb_randr_output_t randr_get_primary_output(void)
{
	xcb_randr_get_output_primary_cookie_t gopc;
	xcb_randr_get_output_primary_reply_t *gopr;
	xcb_randr_output_t out;

	gopc = xcb_randr_get_output_primary(dpy, screen->root);
	gopr = xcb_randr_get_output_primary_reply(dpy, gopc, NULL);

	if (gopr)
		out = gopr->output;
	else
		out = -1;

	free(gopr);
	return out;
}

void warp_pointer(int16_t x, int16_t y)
{
	xcb_warp_pointer(dpy, XCB_NONE, screen->root, 0, 0, 0, 0, x, y);
}

void center_pointer(xcb_rectangle_t rect)
{
	warp_pointer(rect.x + (rect.width / 2), rect.y + (rect.height / 2));
}
