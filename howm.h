#ifndef HOWM_H
#define HOWM_H

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <xcb/xcb_ewmh.h>
#include "config.h"

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

void howm_info(void);
static void setup(void);
static void cleanup(void);
static uint32_t get_colour(char *colour);

#endif
