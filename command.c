#include <stddef.h>
#include <unistd.h>
#include <stdbool.h>
#include <xcb/xcb_ewmh.h>

#include "command.h"
#include "workspace.h"
#include "helper.h"
#include "howm.h"
#include "client.h"
#include "layout.h"
#include "scratchpad.h"

/**
 * @file command.c
 *
 * @author Harvey Hunt
 *
 * @date 2014
 *
 * @brief Commands are bound to keybindings or are executed as a result of a
 * message from IPC.
 */

/**
 * @brief Change the mode of howm.
 *
 * Modes should be thought of in the same way as they are in vi. Different
 * modes mean keypresses cause different actions.
 *
 * @param arg arg->i defines which mode to be selected.
 */
void change_mode(const Arg *arg)
{
	if (arg->i >= (int)END_MODES || arg->i == (int)cur_mode)
		return;
	cur_mode = arg->i;
	log_info("Changing to mode %d", cur_mode);
	howm_info();
}

/**
 * @brief Toggle a client between being in a floating or non-floating state.
 *
 * @param arg Unused.
 */
void toggle_float(const Arg *arg)
{
	UNUSED(arg);
	if (!wss[cw].current)
		return;
	log_info("Toggling floating state of client <%p>", wss[cw].current);
	wss[cw].current->is_floating = !wss[cw].current->is_floating;
	if (wss[cw].current->is_floating && conf.center_floating) {
		wss[cw].current->x = (screen_width / 2) - (wss[cw].current->w / 2);
		wss[cw].current->y = (screen_height - wss[cw].bar_height - wss[cw].current->h) / 2;
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
 * @param arg The amount of pixels that the window's size should be changed by.
 */
void resize_float_width(const Arg *arg)
{
	if (!wss[cw].current || !wss[cw].current->is_floating || (int)wss[cw].current->w + arg->i <= 0)
		return;
	log_info("Resizing width of client <%p> from %d by %d", wss[cw].current, wss[cw].current->w, arg->i);
	wss[cw].current->w += arg->i;
	draw_clients();
}

/**
 * @brief Change the height of a floating client.
 *
 * Negative values will shift the bottom edge of the window to the top. The
 * inverse is true for positive values.
 *
 * @param arg The amount of pixels that the window's size should be changed by.
 */
void resize_float_height(const Arg *arg)
{
	if (!wss[cw].current || !wss[cw].current->is_floating || (int)wss[cw].current->h + arg->i <= 0)
		return;
	log_info("Resizing height of client <%p> from %d to %d", wss[cw].current, wss[cw].current->h, arg->i);
	wss[cw].current->h += arg->i;
	draw_clients();
}

/**
 * @brief Change a floating window's y coordinate.
 *
 * Negative values will move the window up. The inverse is true for positive
 * values.
 *
 * @param arg The amount of pixels that the window should be moved.
 */
void move_float_y(const Arg *arg)
{
	if (!wss[cw].current || !wss[cw].current->is_floating)
		return;
	log_info("Changing y of client <%p> from %d to %d", wss[cw].current, wss[cw].current->y, arg->i);
	wss[cw].current->y += arg->i;
	draw_clients();

}

/**
 * @brief Change a floating window's x coordinate.
 *
 * Negative values will move the window to the left. The inverse is true
 * for positive values.
 *
 * @param arg The amount of pixels that the window should be moved.
 */
void move_float_x(const Arg *arg)
{
	if (!wss[cw].current || !wss[cw].current->is_floating)
		return;
	log_info("Changing x of client <%p> from %d to %d", wss[cw].current, wss[cw].current->x, arg->i);
	wss[cw].current->x += arg->i;
	draw_clients();

}

/**
 * @brief Teleport a floating client's window to a location on the screen.
 *
 * @param arg Which location to teleport the window to.
 */
void teleport_client(const Arg *arg)
{
	if (!wss[cw].current || !wss[cw].current->is_floating
			|| wss[cw].current->is_transient)
		return;

	/* A bit naughty, but it looks nicer- doesn't it?*/
	uint16_t g = wss[cw].current->gap;
	uint16_t w = wss[cw].current->w;
	uint16_t h = wss[cw].current->h;
	uint16_t bh = wss[cw].bar_height;

	switch (arg->i) {
	case TOP_LEFT:
		wss[cw].current->x = g;
		wss[cw].current->y = (conf.bar_bottom ? 0 : bh) + g;
		break;
	case TOP_CENTER:
		wss[cw].current->x = (screen_width - w) / 2;
		wss[cw].current->y = (conf.bar_bottom ? 0 : bh) + g;
		break;
	case TOP_RIGHT:
		wss[cw].current->x = screen_width - w - g - (2 * conf.border_px);
		wss[cw].current->y = (conf.bar_bottom ? 0 : bh) + g;
		break;
	case CENTER:
		wss[cw].current->x = (screen_width - w) / 2;
		wss[cw].current->y = (screen_height - bh - h) / 2;
		break;
	case BOTTOM_LEFT:
		wss[cw].current->x = g;
		wss[cw].current->y = (conf.bar_bottom ? screen_height - bh : screen_height) - h - g - (2 * conf.border_px);
		break;
	case BOTTOM_CENTER:
		wss[cw].current->x = (screen_width / 2) - (w / 2);
		wss[cw].current->y = (conf.bar_bottom ? screen_height - bh : screen_height) - h - g - (2 * conf.border_px);
		break;
	case BOTTOM_RIGHT:
		wss[cw].current->x = screen_width - w - g - (2 * conf.border_px);
		wss[cw].current->y = (conf.bar_bottom ? screen_height - bh : screen_height) - h - g - (2 * conf.border_px);
		break;
	};
	draw_clients();
}

/**
 * @brief Resize the master window of a stack for the current workspace.
 *
 * @param arg The amount to resize the master window by. Treated as a
 * percentage. e.g. arg->i = 5 will increase the master window's size by 5% of
 * it maximum.
 */
void resize_master(const Arg *arg)
{
	/* Resize master only when resizing is visible (i.e. in Stack layouts). */
	if (wss[cw].layout != HSTACK && wss[cw].layout != VSTACK)
		return;

	float change = ((float)arg->i) / 100;

	if (wss[cw].master_ratio + change >= 1
			|| wss[cw].master_ratio + change <= 0.1)
		return;
	log_info("Resizing master_ratio from <%.2f> to <%.2f>", wss[cw].master_ratio, wss[cw].master_ratio + change);
	wss[cw].master_ratio += change;
	arrange_windows();
}

/**
 * @brief Toggle the space reserved for a status bar.
 *
 * @param arg Unused.
 */
void toggle_bar(const Arg *arg)
{
	UNUSED(arg);
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

/**
 * @brief Moves the current window to the master window, when in stack mode.
 *
 * @param arg Unused
 */
void make_master(const Arg *arg)
{
	UNUSED(arg);
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
 * @brief Save last operator, count and motion. These saved values are then
 * used in replay.
 *
 * @param op The last operator.
 * @param type The last type (defined by a motion).
 * @param cnt The last count.
 */
void save_last_ocm(void (*op)(const unsigned int, int), const unsigned int type, int cnt)
{
	rep_state.last_op = op;
	rep_state.last_type = type;
	rep_state.last_cnt = cnt;
	rep_state.last_cmd = NULL;
}

/**
 * @brief Save the last command and argument that was passed to it. These saved
 * values are then used in replay.
 *
 * @param cmd The last command.
 * @param arg The argument passed to the last command.
 */
void save_last_cmd(void (*cmd)(const Arg *arg), const Arg *arg)
{
	rep_state.last_cmd = cmd;
	rep_state.last_arg = arg;
	rep_state.last_op = NULL;
}

/**
 * @brief Replay the last command or operator, complete with the last arguments
 * passed to them.
 *
 * @param arg Unused
 */
void replay(const Arg *arg)
{
	UNUSED(arg);
	if (rep_state.last_cmd)
		rep_state.last_cmd(rep_state.last_arg);
	else
		rep_state.last_op(rep_state.last_type, rep_state.last_cnt);
}

/**
 * @brief Remove a list of clients from howm's delete register stack and paste
 * them after the currently focused window.
 *
 * @param arg Unused
 */
void paste(const Arg *arg)
{
	UNUSED(arg);
	Client *head = stack_pop(&del_reg);
	Client *t, *c = head;

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
 * @brief Change the layout of the current workspace.
 *
 * @param arg A numerical value (arg->i) representing the layout that should be
 * used.
 */
void change_layout(const Arg *arg)
{
	if (arg->i == wss[cw].layout || arg->i >= END_LAYOUT || arg->i < 0)
		return;
	prev_layout = wss[cw].layout;
	wss[cw].layout = arg->i;
	update_focused_client(wss[cw].current);
	log_info("Changed layout from %d to %d", prev_layout,  wss[cw].layout);
}

/**
 * @brief Change to the previous layout.
 *
 * @param arg Unused.
 */
void previous_layout(const Arg *arg)
{
	UNUSED(arg);
	const Arg a = { .i = wss[cw].layout < 1 ? END_LAYOUT - 1 : wss[cw].layout - 1 };

	log_info("Changing to previous layout (%d)", a.i);
	change_layout(&a);
}

/**
 * @brief Change to the next layout.
 *
 * @param arg Unused.
 */
void next_layout(const Arg *arg)
{
	UNUSED(arg);
	const Arg a = { .i = (wss[cw].layout + 1) % END_LAYOUT };

	log_info("Changing to layout (%d)", a.i);
	change_layout(&a);
}

/**
 * @brief Change to the last used layout.
 *
 * @param arg Unused.
 */
void last_layout(const Arg *arg)
{
	UNUSED(arg);

	log_info("Changing to last layout (%d)", prev_layout);
	change_layout(&(Arg){ .i = prev_layout });
}

/**
 * @brief Restart howm.
 *
 * @param arg Unused.
 */
void restart_howm(const Arg *arg)
{
	UNUSED(arg);
	log_warn("Restarting.");
	running = false;
	restart = true;
}

/**
 * @brief Quit howm and set the return value.
 *
 * @param arg The return value that howm will send.
 */
void quit_howm(const Arg *arg)
{
	log_warn("Quitting");
	retval = arg->i;
	running = false;
}

/**
 * @brief Toggle the fullscreen state of the current client.
 *
 * @param arg Unused.
 */
void toggle_fullscreen(const Arg *arg)
{
	UNUSED(arg);
	if (wss[cw].current != NULL)
		set_fullscreen(wss[cw].current, !wss[cw].current->is_fullscreen);
}

/**
 * @brief Focus a client that has an urgent hint.
 *
 * @param arg Unused.
 */
void focus_urgent(const Arg *arg)
{
	UNUSED(arg);
	Client *c;
	unsigned int w;

	for (w = 1; w <= conf.workspaces; w++)
		for (c = wss[w].head; c && !c->is_urgent; c = c->next)
			;
	if (c) {
		log_info("Focusing urgent client <%p> on workspace <%d>", c, w);
		change_ws(&(Arg){.i = w});
		update_focused_client(c);
	}
}

/**
 * @brief Spawns a command.
 */
void spawn(const Arg *arg)
{
	if (fork())
		return;
	if (dpy)
		close(screen->root);
	setsid();
	log_info("Spawning command: %s", (char *)arg->cmd[0]);
	execvp((char *)arg->cmd[0], (char **)arg->cmd);
	log_err("execvp of command: %s failed.", (char *)arg->cmd[0]);
	exit(EXIT_FAILURE);
}

/**
 * @brief Focus the previous workspace.
 *
 * @param arg Unused.
 */
void focus_prev_ws(const Arg *arg)
{
	UNUSED(arg);

	log_info("Focusing previous workspace");
	change_ws(&(Arg){ .i = correct_ws(cw - 1) });
}

/**
 * @brief Focus the last focused workspace.
 *
 * @param arg Unused.
 */
void focus_last_ws(const Arg *arg)
{
	UNUSED(arg);

	log_info("Focusing last workspace");
	change_ws(&(Arg){ .i = last_ws });
}

/**
 * @brief Focus the next workspace.
 *
 * @param arg Unused.
 */
void focus_next_ws(const Arg *arg)
{
	UNUSED(arg);

	log_info("Focusing previous workspace");
	change_ws(&(Arg){ .i = correct_ws(cw + 1) });
}

/**
 * @brief Change to a different workspace and map the correct windows.
 *
 * @param arg arg->i indicates which workspace howm should change to.
 */
void change_ws(const Arg *arg)
{
	Client *c = wss[arg->i].head;

	if ((unsigned int)arg->i > conf.workspaces || arg->i <= 0 || arg->i == cw)
		return;
	last_ws = cw;
	log_info("Changing from workspace <%d> to <%d>.", last_ws, arg->i);
	for (; c; c = c->next)
		xcb_map_window(dpy, c->win);
	for (c = wss[last_ws].head; c; c = c->next)
		xcb_unmap_window(dpy, c->win);
	cw = arg->i;
	update_focused_client(wss[cw].current);

	xcb_ewmh_set_current_desktop(ewmh, 0, cw - 1);
	xcb_ewmh_geometry_t workarea[] = { { 0, conf.bar_bottom ? 0 : wss[cw].bar_height,
				screen_width, screen_height - wss[cw].bar_height } };
	xcb_ewmh_set_workarea(ewmh, 0, LENGTH(workarea), workarea);

	howm_info();
}

