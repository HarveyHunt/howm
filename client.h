#ifndef CLIENT_H
#define CLIENT_H

#include <xcb/xcb.h>
#include <stdbool.h>

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

extern const Rule rules[];

int get_non_tff_count(void);
Client *get_first_non_tff(void);
void change_client_gaps(Client *c, int size);
void kill_client(const int ws, bool arrange);
static void move_down(Client *c);
void move_up(Client *c);
Client *next_client(Client *c);
void update_focused_client(Client *c);
Client *prev_client(Client *c, int ws);
Client *create_client(xcb_window_t w);
void remove_client(Client *c, bool refocus);
Client *find_client_by_win(xcb_window_t w);
static void client_to_ws(Client *c, const int ws, bool follow);
void draw_clients(void);
void change_client_geom(Client *c, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void set_fullscreen(Client *c, bool fscr);
void set_urgent(Client *c, bool urg);
void apply_rules(Client *c);
void move_client(int cnt, bool up);

#endif
