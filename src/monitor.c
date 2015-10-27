#include <stdlib.h>

#include "monitor.h"
#include "helper.h"
#include "howm.h"
#include "workspace.h"
#include "xcb_help.h"

/**
 * @file monitor.c
 *
 * @author Harvey Hunt
 *
 * @date 2015
 *
 * @brief A monitor stores workspaces. The user can have multiple monitors.
 */

/**
 * @brief Allocate memory for a monitor and update global state.
 *
 * @param rect A rectangle representing the size of the monitor.
 *
 * @return An initialised monitor.
 */
monitor_t *create_monitor(xcb_rectangle_t rect)
{
	monitor_t *m = calloc(1, sizeof(monitor_t));

	if (!m) {
		log_err("Can't allocate memory for monitor");
		exit(EXIT_FAILURE);
	}
	m->rect = rect;

	if (mon == NULL) {
		mon_head = mon = mon_tail = m;
	} else {
		mon_tail->next = m;
		m->prev = mon_tail;
		mon_tail = m;
	}

	log_info("Added monitor <%d> with dimensions: {%d, %d, %d, %d}",
			monitor_to_index(m), m->rect.x, m->rect.y,
			m->rect.width, m->rect.height);

	mon_cnt++;

	return m;
}

/**
 * @brief Remove a monitor and all of its workspaces.
 *
 * @param m The monitor to be removed.
 */
void remove_monitor(monitor_t *m)
{
	monitor_t *next = m->next;
	monitor_t *prev = m->prev;

	log_info("Removing monitor <%d>", monitor_to_index(m));

	mon_cnt--;

	while (m->ws_head)
		remove_ws(m, m->ws_head);

	if (prev)
		prev->next = next;
	if (next)
		next->prev = prev;
	if (m == mon_head)
		mon_head = next;
	if (m == mon_tail)
		mon_tail = prev;
	if (m == mon)
		mon = prev ? prev : next;

	/* TODO: Maybe we'll need to refocus? */

	free(m);
}

/**
 * @brief Set a monitor as the focused monitor.
 *
 * @param m The monitor to be focused.
 */
void focus_monitor(monitor_t *m)
{
	if (!m || mon == m)
		return;

	mon = m;

	log_info("Focusing monitor <%d>", monitor_to_index(mon));

	if (mon->ws && mon->ws->c)
		xcb_set_input_focus(dpy, XCB_INPUT_FOCUS_POINTER_ROOT, mon->ws->c->win,
			    XCB_CURRENT_TIME);

	ewmh_set_current_workspace();
}

/**
 * @brief Find and return a monitor's index in the monitor list.
 *
 * @param m The monitor to search for.
 *
 * @return The index of the monitor in the monitor list.
 */
uint32_t monitor_to_index(const monitor_t *m)
{
	monitor_t *om;
	uint32_t i = 0;

	for (om = mon_head; om != NULL; om = om->next, i++)
		if (om == m)
			return i;
	return 0;
}

/**
 * @brief Convert a monitor's index in a monitor list to an index.
 *
 * @param index The index to search for.
 *
 * @return The monitor stored at the index of the monitors list.
 */
monitor_t *index_to_monitor(uint32_t index)
{
	monitor_t *m;

	for (m = mon_head; index > 0 && m != NULL; m = m->next, index--)
		;

	return m;
}

/**
 * @brief Create a single monitor for use with default X11.
 */
static void scan_x11_monitor(void)
{
	monitor_t *m = create_monitor((xcb_rectangle_t) { 0, 0, screen_width, screen_height });

	add_ws(m);
}

/**
 * @brief Detect and initialise monitors for each Xrandr output.
 *
 * @return True if Xrandr is detected and monitors are created.
 */
static inline bool scan_xrandr_monitors(void)
{
	return false;
}

/**
 * @brief Convert a point to a monitor that it is within.
 *
 * @param point The point to be converted to a monitor.
 *
 * @return The monitor containing the point, or NULL.
 */
monitor_t *point_to_monitor(xcb_point_t point)
{
	monitor_t *m = mon_head;

	for (; m != NULL; m = m->next) {
		if (point.x >= m->rect.x && point.x < (m->rect.width + m->rect.x)
				&& point.y >= m->rect.y
				&& point.y < (m->rect.height + m->rect.y))
			return m;
	}

	return NULL;
}

/**
 * @brief Initialise a monitor for each supported screen.
 */
void scan_monitors(void)
{
	if (!scan_xrandr_monitors())
		scan_x11_monitor();
}
