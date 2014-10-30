#ifndef HOWM_H
#define HOWM_H

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <xcb/xcb_ewmh.h>
#include "op.h"
#include "command.h"
#include "client.h"
#include "config.h"

/**
 * @file howm.h
 *
 * @author Harvey Hunt
 *
 * @date 2014
 *
 * @brief howm
 */

struct config {
	unsigned int workspaces;
	bool focus_mouse;
	bool focus_mouse_click;
	bool follow_move;
	uint16_t border_px;
	char *border_focus;
	char *border_unfocus;
	char *border_prev_focus;
	char *border_urgent;
	uint16_t bar_height;
	bool bar_bottom;
	uint16_t op_gap_size;
	bool center_floating;
	bool zoom_gap;
	float master_ratio;
	unsigned int log_level;
	unsigned int default_workspace;
	unsigned int ws_def_layout;
	uint16_t float_spawn_width;
	uint16_t float_spawn_height;
	char *howm_path;
	unsigned int delete_register_size;
	uint16_t scratchpad_height;
	uint16_t scratchpad_width;
	char *sock_path;
	unsigned int ipc_buf_size;
};

enum states { OPERATOR_STATE, COUNT_STATE, MOTION_STATE, END_STATE };

extern int numlockmask;
extern int retval;
extern int last_ws;
extern int prev_layout;
extern int cw;
extern xcb_connection_t *dpy;
extern uint32_t border_focus;
extern uint32_t border_unfocus;
extern uint32_t border_prev_focus;
extern uint32_t border_urgent;
extern unsigned int cur_mode;
extern uint16_t screen_height;
extern uint16_t screen_width;
extern int cur_state;

extern xcb_screen_t *screen;
extern xcb_ewmh_connection_t *ewmh;
extern bool running;
extern bool restart;

extern Workspace *wss;

extern struct config conf;

extern const char *WM_ATOM_NAMES[];
extern xcb_atom_t wm_atoms[];

void howm_info(void);

#endif
