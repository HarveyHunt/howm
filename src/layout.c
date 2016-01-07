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

static void stack(monitor_t *m);
static void grid(monitor_t *m);
static void zoom(monitor_t *m);

static void(*layout_handler[]) (monitor_t *m) = {
	[GRID] = grid,
	[ZOOM] = zoom,
	[HSTACK] = stack,
	[VSTACK] = stack
};

/**
 * @brief Call the appropriate layout handler for each layout.
 *
 * @param m The monitor to be arranged.
 */
void arrange_windows(monitor_t *m)
{
	if (!m->ws->head)
		return;
	log_debug("Arranging windows");
	layout_handler[m->ws->head->next ? m->ws->layout : ZOOM](mon);
	howm_info();
}

/**
 * @brief Arrange the windows into a grid layout.
 *
 * @param m The monitor to be arranged.
 */
static void grid(monitor_t *m)
{
	int n = get_non_tff_count();
	client_t *c = NULL;
	int cols, rows, i = -1, col_cnt = 0, row_cnt = 0;
	uint16_t col_w;
	uint16_t client_y = conf.bar_bottom ? m->rect.y : m->rect.y + m->ws->bar_height;
	uint16_t col_h = m->rect.height - m->ws->bar_height;

	if (n <= 1) {
		zoom(mon);
		return;
	}

	log_info("Arranging %d clients in grid layout", n);

	for (cols = 1; cols <= n / 2; cols++)
		if (cols * cols >= n)
			break;

	rows = n / cols;
	col_w = m->rect.width / cols;
	for (c = m->ws->head; c; c = c->next) {
		if (FFT(c))
			continue;
		else
			i++;

		if (cols - (n % cols) < (i / rows) + 1)
			rows = n / cols + 1;
		change_client_geom(c, (col_cnt * col_w) + m->rect.x,
				client_y + (row_cnt * col_h / rows),
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
 *
 * @param m The monitor to be arranged.
 */
static void zoom(monitor_t *m)
{
	client_t *c;

	log_info("Arranging clients in zoom format");
	/* When zoom is called because there aren't enough clients for other
	 * layouts to work, draw a border to be consistent with other layouts.
	 * */
	if (m->ws->layout != ZOOM && !m->ws->head->is_fullscreen)
		set_border_width(m->ws->head->win, conf.border_px);

	for (c = m->ws->head; c; c = c->next)
		if (!FFT(c))
			change_client_geom(c, m->rect.x, conf.bar_bottom
					? m->rect.y : m->rect.y + m->ws->bar_height,
					m->rect.width, m->rect.height - m->ws->bar_height);
	draw_clients();
}

/**
 * @brief Arrange the windows in a stack, whether that be horizontal or
 * vertical is decided by the current_layout.
 *
 * @param m The monitor to be arranged.
 */
static void stack(monitor_t *m)
{
	client_t *c = get_first_non_tff();
	bool vert = (m->ws->layout == VSTACK);
	uint16_t h = m->rect.height - m->ws->bar_height;
	uint16_t w = m->rect.width;
	int n = get_non_tff_count();
	uint16_t client_x = m->rect.x, client_span = 0;
	uint16_t client_y = conf.bar_bottom ? m->rect.y : m->rect.y + m->ws->bar_height;
	uint16_t ms = (vert ? w : h) * m->ws->master_ratio;
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
		zoom(mon);
		return;
	}

	/* TODO: Need to take into account when this has remainders. */
	client_span = (span / (n - 1));

	log_info("Arranging %d clients in %sstack layout", n, vert ? "v" : "h");
	if (vert) {
		change_client_geom(c, m->rect.x, client_y,
			    ms, span);
	} else {
		change_client_geom(c, m->rect.x, conf.bar_bottom ? m->rect.y : m->rect.y + m->ws->bar_height,
			span, ms);
	}

	for (c = c->next; c; c = c->next) {
		if (FFT(c))
			continue;
		if (vert) {
			change_client_geom(c, m->rect.x + ms, client_y,
				    m->rect.width - ms,
				    client_span);
			client_y += client_span;
		} else {
			change_client_geom(c, client_x, m->rect.y + ms,
				    client_span,
				    m->rect.height - m->ws->bar_height - ms);
			client_x += client_span;
		}
	}
	draw_clients();
}

/**
 * @brief Change the layout of the current workspace.
 *
 * @param m The monitor to be arranged.
 * @param layout Represents the layout that should be used.
 */
void change_layout(monitor_t *m, const int layout)
{
	if (layout == m->ws->layout || layout >= END_LAYOUT || layout < ZOOM)
		return;
	m->ws->layout = layout;
	update_focused_client(m->ws->c);
	log_info("Changed layout from %d to %d", m->ws->last_layout,  m->ws->layout);
	m->ws->last_layout = m->ws->layout;
}

/**
 * @brief Change to the previous layout.
 *
 * @ingroup commands
 * @param m The monitor to be arranged.
 */
void prev_layout(monitor_t *m)
{
	int i = m->ws->layout < 1 ? END_LAYOUT - 1 : m->ws->layout - 1;

	log_info("Changing to previous layout (%d)", i);
	change_layout(m, i);
}

/**
 * @brief Change to the next layout.
 *
 * @param m The monitor to be arranged.
 * @ingroup commands
 */
void next_layout(monitor_t *m)
{
	int i = (m->ws->layout + 1) % END_LAYOUT;

	log_info("Changing to layout (%d)", i);
	change_layout(m, i);
}

/**
 * @brief Change to the last used layout.
 *
 * @param m The monitor to be arranged.
 * @ingroup commands
 */
void last_layout(monitor_t *m)
{
	log_info("Changing to last layout (%d)", m->ws->last_layout);
	change_layout(m, m->ws->last_layout);
}
