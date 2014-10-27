#include "layout.h"

/**
 * @brief Call the appropriate layout handler for each layout.
 */
void arrange_windows(void)
{
	if (!wss[cw].head)
		return;
	log_debug("Arranging windows");
	layout_handler[wss[cw].head->next ? wss[cw].layout : ZOOM]();
	howm_info();
}

/**
 * @brief Arrange the windows into a grid layout.
 */
void grid(void)
{
	int n = get_non_tff_count();
	Client *c = NULL;
	int cols, rows, i = -1, col_cnt = 0, row_cnt = 0;
	uint16_t col_w;
	uint16_t client_y = BAR_BOTTOM ? 0 : wss[cw].bar_height;
	uint16_t col_h = screen_height - wss[cw].bar_height;

	if (n <= 1) {
		zoom();
		return;
	}

	log_info("Arranging %d clients in grid layout", n);

	for (cols = 0; cols <= n / 2; cols++)
		if (cols * cols >= n)
			break;
	rows = n / cols;
	col_w = screen_width / cols;
	for (c = wss[cw].head; c; c = c->next) {
		if (FFT(c))
			continue;
		else
			i++;

		if (cols - (n % cols) < (i / rows) + 1)
			rows = n / cols + 1;
		change_client_geom(c, col_cnt * col_w, client_y + (row_cnt * col_h / rows),
				col_w, (col_h / rows));
		if (++row_cnt >= rows) {
			row_cnt = 0;
			col_cnt++;
		}
	}
	draw_clients();
}

/**
 * @brief Have one window at a time taking up the entire screen.
 *
 * Sets the geometry of each window in order for the windows to be rendered to
 * take up the entire screen.
 */
void zoom(void)
{
	Client *c;

	log_info("Arranging clients in zoom format");
	/* When zoom is called because there aren't enough clients for other
	 * layouts to work, draw a border to be consistent with other layouts.
	 * */
	if (wss[cw].layout != ZOOM && !wss[cw].head->is_fullscreen)
		set_border_width(wss[cw].head->win, BORDER_PX);

	for (c = wss[cw].head; c; c = c->next)
		if (!FFT(c))
			change_client_geom(c, 0, BAR_BOTTOM ? 0 : wss[cw].bar_height,
					screen_width, screen_height - wss[cw].bar_height);
	draw_clients();
}

/**
 * @brief Arrange the windows in a stack, whether that be horizontal or
 * vertical is decided by the current_layout.
 */
void stack(void)
{
	Client *c = get_first_non_tff();
	bool vert = (wss[cw].layout == VSTACK);
	uint16_t h = screen_height - wss[cw].bar_height;
	uint16_t w = screen_width;
	int n = get_non_tff_count();
	uint16_t client_x = 0, client_span = 0;
	uint16_t client_y = BAR_BOTTOM ? 0 : wss[cw].bar_height;
	uint16_t ms = (vert ? w : h) * wss[cw].master_ratio;
	/* The size of the direction the clients will be stacked in. e.g.
	 *
	 *+---------------------------+--------------+   +
	 *|                           |              |   |
	 *|                           |              |   |
	 *|                           |              |   |
	 *|                           |              |   |
	 *|                           |              |   | Span for vert stack
	 *|                           +--------------+   |
	 *|                           |              |   |
	 *|                           |              |   |
	 *|                           |              |   |
	 *|                           |              |   |
	 *|                           +--------------+   |
	 *|                           |              |   |
	 *|                           |              |   |
	 *|                           |              |   |
	 *|                           |              |   |
	 *+---------------------------+--------------+   v
	 */
	uint16_t span = vert ? h : w;

	if (n <= 1) {
		zoom();
		return;
	}

	/* TODO: Need to take into account when this has remainders. */
	client_span = (span / (n - 1));

	log_info("Arranging %d clients in %sstack layout", n, vert ? "v" : "h");
	if (vert) {
		change_client_geom(c, 0, client_y,
			    ms, span);
	} else {
		change_client_geom(c, 0, BAR_BOTTOM ? 0 : wss[cw].bar_height,
			span, ms);
	}

	for (c = c->next; c; c = c->next) {
		if (FFT(c))
			continue;
		if (vert) {
			change_client_geom(c, ms, client_y,
				    screen_width - ms,
				    client_span);
			client_y += client_span;
		} else {
			change_client_geom(c, client_x, ms,
				    client_span,
				    screen_height - wss[cw].bar_height - ms);
			client_x += client_span;
		}
	}
	draw_clients();
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

