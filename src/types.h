#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <stdint.h>
#include <xcb/randr.h>
#include <xcb/xproto.h>

/**
 * @file types.h
 *
 * @author Harvey Hunt
 *
 * @date 2015
 *
 * @brief howm
 */

/**
 * @brief Represents a client that is being handled by howm.
 *
 * All the attributes that are needed by howm for a client are stored here.
 */
typedef struct client_t client_t;
struct client_t {
	client_t *next; /**< Clients are stored in a linked list-
					* this represents the client after this one. */
	bool is_fullscreen; /**< Is the client fullscreen? */
	bool is_floating; /**< Is the client floating? */
	bool is_transient; /**< Is the client transient?
					* Defined at: http://standards.freedesktop.org/wm-spec/wm-spec-latest.html*/
	bool is_urgent; /**< This is set by a client that wants focus for some reason. */
	xcb_window_t win; /**< The window that this client represents. */
	xcb_rectangle_t rect; /**< The size and location of the client. */
	uint16_t gap; /**< The size of the useless gap between this client and
			the others. */
};

/**
 * @brief Represents a workspace, which stores clients.
 *
 * Clients are stored as a linked list. Changing to a different workspace will
 * cause different clients to be rendered on the screen.
 *
 * Workspaces are also stored as a linked list.
 */
typedef struct workspace_t workspace_t;
struct workspace_t {
	int layout; /**< The current layout of the WS, as defined in the
				* layout enum. */
	unsigned int client_cnt; /**< The amount of clients on this workspace. */
	uint16_t gap; /**< The size of the useless gap between windows for this workspace. */
	float master_ratio; /**< The ratio of the size of the master window
				 compared to the screen's size. */
	uint16_t bar_height; /**< The height of the space left for a bar. Stored
			      here so it can be toggled per ws. */
	client_t *head; /**< The start of the linked list. */
	client_t *prev_foc; /**< The last focused client. This is seperate to
				* the linked list structure. */
	client_t *c; /**< The client that is currently in focus. */
	workspace_t *next; /**< The next workspace in the linked list. */
	workspace_t *prev; /**< The prev workspace in the linked list. */
	unsigned int last_layout; /**< The last layout used. */
};

/**
 * @brief Represents a monitor.
 *
 * Each monitor has its own workspaces. When the user is not using a
 * multimonitor setup, we still create a single monitor.
 */
typedef struct monitor_t monitor_t;
struct monitor_t {
	unsigned int workspace_cnt; /**< The amount of workspaces on this monitor. */
	workspace_t *ws; /**< The currently focused workspace. */
	workspace_t *ws_head; /**< The first workspace. */
	workspace_t *ws_tail; /**< The last workspace. */
	workspace_t *last_ws; /**< The last workspace to be focused. */
	monitor_t *next; /**< The next monitor. */
	monitor_t *prev; /**< The previous monitor. */
	xcb_rectangle_t rect; /**< The size and location of the monitor. */
	xcb_randr_output_t output; /**< The ID of the randr output. */
};

typedef struct {
	monitor_t *mon;
	workspace_t *ws;
	client_t *c;
} location_t;

#endif
