#ifndef CLIENT_H
#define CLIENT_H

#include <xcb/xcb.h>
#include <stdbool.h>
#include "types.h"

int get_non_tff_count(void);
Client *get_first_non_tff(void);
void change_client_gaps(Client *c, int size);
void kill_client(const int ws, bool arrange);
void move_up(Client *c);
Client *next_client(Client *c);
void update_focused_client(Client *c);
Client *prev_client(Client *c, int ws);
Client *create_client(xcb_window_t w);
void remove_client(Client *c, bool refocus);
Client *find_client_by_win(xcb_window_t w);
void client_to_ws(Client *c, const int ws, bool follow);
void draw_clients(void);
void change_client_geom(Client *c, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void set_fullscreen(Client *c, bool fscr);
void set_urgent(Client *c, bool urg);
void apply_rules(Client *c);
void move_client(int cnt, bool up);

#endif
