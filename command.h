#ifndef COMMAND_H
#define COMMAND_H

#include <xcb/xcb.h>
#include "types.h"

/**
 * @file command.h
 *
 * @author Harvey Hunt
 *
 * @date 2014
 *
 * @brief howm
 */

enum teleport_locations { TOP_LEFT, TOP_CENTER, TOP_RIGHT, CENTER, BOTTOM_LEFT, BOTTOM_CENTER, BOTTOM_RIGHT };
enum modes { NORMAL, FOCUS, FLOATING, END_MODES };

void (*operator_func)(const unsigned int type, int cnt);

void teleport_client(const int direction);
void move_current_down(void);
void move_current_up(void);
void focus_next_client(void);
void focus_prev_client(void);
void current_to_ws(const int ws);
void toggle_float(void);
void resize_float_width(const int dw);
void resize_float_height(const int dh);
void move_float_y(const int dy);
void move_float_x(const int dx);
void toggle_fullscreen(void);
void focus_urgent(void);
void make_master(void);
void toggle_bar(void);
void resize_master(const int ds);
void focus_next_ws(void);
void focus_prev_ws(void);
void focus_last_ws(void);
void change_ws(const int ws);
void change_mode(const int mode);
void quit_howm(const int exit_status);
void restart_howm(void);
void paste(void);
void change_layout(const int layout);
void next_layout(void);
void prev_layout(void);
void last_layout(void);
void spawn(char * cmd[]);
void count(const int cnt);
void motion(char *target);
void send_to_scratchpad(void);
void get_from_scratchpad(void);

#endif
