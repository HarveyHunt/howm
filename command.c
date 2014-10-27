#include "command.h"

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
static void toggle_float(const Arg *arg)
{
	UNUSED(arg);
	if (!wss[cw].current)
		return;
	log_info("Toggling floating state of client <%p>", wss[cw].current);
	wss[cw].current->is_floating = !wss[cw].current->is_floating;
	if (wss[cw].current->is_floating && CENTER_FLOATING) {
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
static void resize_float_width(const Arg *arg)
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
static void resize_float_height(const Arg *arg)
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
static void move_float_y(const Arg *arg)
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
static void move_float_x(const Arg *arg)
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
static void teleport_client(const Arg *arg)
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
		wss[cw].current->y = (BAR_BOTTOM ? 0 : bh) + g;
		break;
	case TOP_CENTER:
		wss[cw].current->x = (screen_width - w) / 2;
		wss[cw].current->y = (BAR_BOTTOM ? 0 : bh) + g;
		break;
	case TOP_RIGHT:
		wss[cw].current->x = screen_width - w - g - (2 * BORDER_PX);
		wss[cw].current->y = (BAR_BOTTOM ? 0 : bh) + g;
		break;
	case CENTER:
		wss[cw].current->x = (screen_width - w) / 2;
		wss[cw].current->y = (screen_height - bh - h) / 2;
		break;
	case BOTTOM_LEFT:
		wss[cw].current->x = g;
		wss[cw].current->y = (BAR_BOTTOM ? screen_height - bh : screen_height) - h - g - (2 * BORDER_PX);
		break;
	case BOTTOM_CENTER:
		wss[cw].current->x = (screen_width / 2) - (w / 2);
		wss[cw].current->y = (BAR_BOTTOM ? screen_height - bh : screen_height) - h - g - (2 * BORDER_PX);
		break;
	case BOTTOM_RIGHT:
		wss[cw].current->x = screen_width - w - g - (2 * BORDER_PX);
		wss[cw].current->y = (BAR_BOTTOM ? screen_height - bh : screen_height) - h - g - (2 * BORDER_PX);
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
static void resize_master(const Arg *arg)
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
static void toggle_bar(const Arg *arg)
{
	UNUSED(arg);
	if (wss[cw].bar_height == 0 && BAR_HEIGHT > 0) {
		wss[cw].bar_height = BAR_HEIGHT;
		log_info("Toggled bar to shown");
	} else if (wss[cw].bar_height == BAR_HEIGHT) {
		wss[cw].bar_height = 0;
		log_info("Toggled bar to hidden");
	} else {
		return;
	}
	xcb_ewmh_geometry_t workarea[] = { { 0, BAR_BOTTOM ? 0 : wss[cw].bar_height,
				screen_width, screen_height - wss[cw].bar_height } };
	xcb_ewmh_set_workarea(ewmh, 0, LENGTH(workarea), workarea);
	arrange_windows();
}

/**
 * @brief Moves the current window to the master window, when in stack mode.
 *
 * @param arg Unused
 */
static void make_master(const Arg *arg)
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
static void save_last_ocm(void (*op)(const unsigned int, int), const unsigned int type, int cnt)
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
static void save_last_cmd(void (*cmd)(const Arg *arg), const Arg *arg)
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
static void replay(const Arg *arg)
{
	UNUSED(arg);
	if (rep_state.last_cmd)
		rep_state.last_cmd(rep_state.last_arg);
	else
		rep_state.last_op(rep_state.last_type, rep_state.last_cnt);
}

