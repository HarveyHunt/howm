#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <stdint.h>
#include <xcb/xcb.h>

/**
 * @file types.h
 *
 * @author Harvey Hunt
 *
 * @date 2014
 *
 * @brief howm
 */

/**
 * @brief Represents a rule that is applied to a client upon it starting.
 */
typedef struct {
	const char *class; /**<	The class or name of the client. */
	int ws; /**<  The workspace that the client should be spawned
				on (0 means current workspace). */
	bool follow; /**< If the client is spawned on another ws, shall we follow? */
	bool is_floating; /**< Spawn the client in a floating state? */
	bool is_fullscreen; /**< Spawn the client in a fullscreen state? */
} Rule;

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
	uint16_t x; /**< The x coordinate of the client. */
	uint16_t y; /**< The y coordinate of the client. */
	uint16_t w; /**< The width of the client. */
	uint16_t h; /**< The height of the client. */
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

/**
 * @brief Represents an argument.
 *
 * Used to hold data that is sent as a parameter to a function when called as a
 * result of a keypress.
 */
typedef union {
	const char * const * const cmd; /**< Represents a command that will be called by a shell.  */
	int i; /**< Usually used for specifying workspaces or clients. */
} Arg;

typedef struct {
	char *name; /**< The function's name. */
	void (*func)(const Arg *); /**< The function to be called when a command
				     comes in from the socket. */
	void (*operator)(const unsigned int type, const int cnt); /**< The
			operator to be called when a command comes in from
			the socket. */
	int argc; /**< The amount of args this command expects. */
	int arg_type; /**< The argument's type for commands that use the union Arg. */
} Command;

/**
 * @brief Represents a button.
 *
 * Allows the mapping of a button to a function, as is done with the Key struct
 * for keys.
 */
typedef struct {
	int mod; /**< The mask of the modifiers pressed.  */
	short int button; /**< The button that was pressed. */
	void (*func)(const Arg *); /**< The function to be called when the
					* button is pressed. */
	const Arg arg; /**< The argument passed to the above function. */
} Button;

#endif
