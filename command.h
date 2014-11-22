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

/**
 * @brief Represents the last command (and its arguments) or the last
 * combination of operator, count and motion (ocm).
 */
struct replay_state {
	void (*last_op)(const unsigned int type, int cnt); /**< The last operator to be called. */
	void (*last_cmd)(const Arg *arg); /**< The last command to be called. */
	const Arg *last_arg; /**< The last argument, passed to the last command. */
	unsigned int last_type; /**< The value determine by the last motion
				(workspace, client etc).*/
	int last_cnt; /**< The last count passed to the last operator function. */
};


enum teleport_locations { TOP_LEFT, TOP_CENTER, TOP_RIGHT, CENTER, BOTTOM_LEFT, BOTTOM_CENTER, BOTTOM_RIGHT };
enum modes { NORMAL, FOCUS, FLOATING, END_MODES };

struct replay_state rep_state;
void (*operator_func)(const unsigned int type, int cnt);

void teleport_client(const Arg *arg);
void save_last_ocm(void (*op) (const unsigned int, int), const unsigned int type, int cnt);
void save_last_cmd(void (*cmd)(const Arg *), const Arg *arg);
void move_current_down(const Arg *arg);
void move_current_up(const Arg *arg);
void focus_next_client(const Arg *arg);
void focus_prev_client(const Arg *arg);
void current_to_ws(const Arg *arg);
void toggle_float(const Arg *arg);
void resize_float_width(const Arg *arg);
void resize_float_height(const Arg *arg);
void move_float_y(const Arg *arg);
void move_float_x(const Arg *arg);
void toggle_fullscreen(const Arg *arg);
void focus_urgent(const Arg *arg);
void send_to_scratchpad(const Arg *arg);
void get_from_scratchpad(const Arg *arg);
void make_master(const Arg *arg);
void toggle_bar(const Arg *arg);
void resize_master(const Arg *arg);
void focus_next_ws(const Arg *arg);
void focus_prev_ws(const Arg *arg);
void focus_last_ws(const Arg *arg);
void change_ws(const Arg *arg);
void change_mode(const Arg *arg);
void replay(const Arg *arg);
void quit_howm(const Arg *arg);
void restart_howm(const Arg *arg);
void paste(const Arg *arg);
void change_layout(const Arg *arg);
void next_layout(const Arg *arg);
void previous_layout(const Arg *arg);
void last_layout(const Arg *arg);
void spawn(const Arg *arg);
void count(const Arg *arg);
void motion(const Arg *arg);

#endif
