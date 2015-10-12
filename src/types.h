#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <stdint.h>
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
typedef struct Client {
	struct Client *next; /**< Clients are stored in a linked list-
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
} Client;

/**
 * @brief Represents a workspace, which stores clients.
 *
 * Clients are stored as a linked list. Changing to a different workspace will
 * cause different clients to be rendered on the screen.
 */
typedef struct {
	int layout; /**< The current layout of the WS, as defined in the
				* layout enum. */
	unsigned int client_cnt; /**< The amount of clients on this workspace. */
	uint16_t gap; /**< The size of the useless gap between windows for this workspace. */
	float master_ratio; /**< The ratio of the size of the master window
				 compared to the screen's size. */
	uint16_t bar_height; /**< The height of the space left for a bar. Stored
			      here so it can be toggled per ws. */
	Client *head; /**< The start of the linked list. */
	Client *prev_foc; /**< The last focused client. This is seperate to
				* the linked list structure. */
	Client *current; /**< The client that is currently in focus. */
} Workspace;

#endif
