#ifndef WORKSPACE_H
#define WORKSPACE_H

/**
 * @brief Represents a workspace, which stores clients.
 *
 * Clients are stored as a linked list. Changing to a different workspace will
 * cause different clients to be rendered on the screen.
 */
typedef struct {
	int layout; /**< The current layout of the WS, as defined in the
				* layout enum. */
	int client_cnt; /**< The amount of clients on this workspace. */
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

static void kill_ws(const int ws);
static int correct_ws(int ws);

extern Workspace wss[];

#endif

