#include <stddef.h>
#include <stdint.h>

#include "client.h"
#include "helper.h"
#include "howm.h"
#include "layout.h"
#include "types.h"
#include "xcb_help.h"

/**
 * @file layout.c
 *
 * @author Harvey Hunt
 *
 * @date 2015
 *
 * @brief All of howm's layouts (as well as layout handler) are implemented
 * here.
 */

static void stack(void);
static void grid(void);
static void zoom(void);

static void(*layout_handler[]) (void) = {
	[GRID] = grid,
	[ZOOM] = zoom,
	[HSTACK] = stack,
	[VSTACK] = stack
};

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
static void grid(void)
{
	int n = get_non_tff_count();
	client_t *c = NULL;
	int cols, rows, i = -1, col_cnt = 0, row_cnt = 0;
	uint16_t col_w;
	uint16_t client_y = conf.bar_bottom ? 0 : wss[cw].bar_height;
	uint16_t col_h = screen_height - wss[cw].bar_height;

	if (n <= 1) {
		zoom();
		return;
	}

	log_info("Arranging %d clients in grid layout", n);

	for (cols = 1; cols <= n / 2; cols++)
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
static void zoom(void)
{
	client_t *c;

	log_info("Arranging clients in zoom format");
	/* When zoom is called because there aren't enough clients for other
	 * layouts to work, draw a border to be consistent with other layouts.
	 * */
	if (wss[cw].layout != ZOOM && !wss[cw].head->is_fullscreen)
		set_border_width(wss[cw].head->win, conf.border_px);

	for (c = wss[cw].head; c; c = c->next)
		if (!FFT(c))
			change_client_geom(c, 0, conf.bar_bottom ? 0 : wss[cw].bar_height,
					screen_width, screen_height - wss[cw].bar_height);
	draw_clients();
}

/**
 * @brief Arrange the windows in a stack, whether that be horizontal or
 * vertical is decided by the current_layout.
 */
static void stack(void)
{
	client_t *c = get_first_non_tff();
	bool vert = (wss[cw].layout == VSTACK);
	uint16_t h = screen_height - wss[cw].bar_height;
	uint16_t w = screen_width;
	int n = get_non_tff_count();
	uint16_t client_x = 0, client_span = 0;
	uint16_t client_y = conf.bar_bottom ? 0 : wss[cw].bar_height;
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
		change_client_geom(c, 0, conf.bar_bottom ? 0 : wss[cw].bar_height,
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
 * @param layout Represents the layout that should be used.
 */
void change_layout(const int layout)
{
	if (layout == wss[cw].layout || layout >= END_LAYOUT || layout < ZOOM)
		return;
	wss[cw].layout = layout;
	update_focused_client(wss[cw].current);
	log_info("Changed layout from %d to %d", previous_layout,  wss[cw].layout);
	previous_layout = wss[cw].layout;
}

/**
 * @brief Change to the previous layout.
 *
 * @ingroup commands
 */
void prev_layout(void)
{
	int i = wss[cw].layout < 1 ? END_LAYOUT - 1 : wss[cw].layout - 1;

	log_info("Changing to previous layout (%d)", i);
	change_layout(i);
}

/**
 * @brief Change to the next layout.
 *
 * @ingroup commands
 */
void next_layout(void)
{
	int i = (wss[cw].layout + 1) % END_LAYOUT;

	log_info("Changing to layout (%d)", i);
	change_layout(i);
}

/**
 * @brief Change to the last used layout.
 *
 * @ingroup commands
 */
void last_layout(void)
{
	log_info("Changing to last layout (%d)", previous_layout);
	change_layout(previous_layout);
}
