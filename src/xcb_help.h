#ifndef XCB_HELP_H
#define XCB_HELP_H

#include <stdint.h>
#include <xcb/xproto.h>

#include "types.h"

/**
 * @file xcb_help.h
 *
 * @author Harvey Hunt
 *
 * @date 2015
 *
 * @brief howm
 */

/** The remove action for a WM_STATE request. */
#define _NET_WM_STATE_REMOVE 0
/** The add action for a WM_STATE request. */
#define _NET_WM_STATE_ADD 1
/** The toggle action for a WM_STATE request. */
#define _NET_WM_STATE_TOGGLE 2

enum net_atom_enum { NET_WM_STATE_FULLSCREEN, NET_SUPPORTED, NET_WM_STATE,
	NET_ACTIVE_WINDOW };
enum wm_atom_enum { WM_DELETE_WINDOW, WM_PROTOCOLS };


void elevate_window(xcb_window_t win);
void move_resize(xcb_window_t win, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void set_border_width(xcb_window_t win, uint16_t w);
void get_atoms(const char **names, xcb_atom_t *atoms);
void check_other_wm(void);
void focus_window(xcb_window_t win);
void grab_buttons(client_t *c);
void delete_win(xcb_window_t win);
void setup_ewmh(void);
void ewmh_process_wm_state(client_t *c, xcb_atom_t a, int action);

#endif
