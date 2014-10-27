#ifndef CLIENT_H
#define CLIENT_H

static void teleport_client(const Arg *arg);
static void change_client_gaps(Client *c, int size);
static void change_gaps(const unsigned int type, int cnt, int size);
static void move_current_down(const Arg *arg);
static void move_current_up(const Arg *arg);
static void kill_client(const int ws, bool arrange);
static void move_down(Client *c);
static void move_up(Client *c);
static Client *next_client(Client *c);
static void focus_next_client(const Arg *arg);
static void focus_prev_client(const Arg *arg);
static void update_focused_client(Client *c);
static Client *prev_client(Client *c, int ws);
static Client *create_client(xcb_window_t w);
static void remove_client(Client *c, bool refocus);
static Client *find_client_by_win(xcb_window_t w);
static void client_to_ws(Client *c, const int ws, bool follow);
static void current_to_ws(const Arg *arg);
static void draw_clients(void);
static void change_client_geom(Client *c, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
static void toggle_float(const Arg *arg);
static void resize_float_width(const Arg *arg);
static void resize_float_height(const Arg *arg);
static void move_float_y(const Arg *arg);
static void move_float_x(const Arg *arg);
static void make_master(const Arg *arg);
static void set_fullscreen(Client *c, bool fscr);
static void set_urgent(Client *c, bool urg);
static void toggle_fullscreen(const Arg *arg);
static void focus_urgent(const Arg *arg);
static void send_to_scratchpad(const Arg *arg);
static void get_from_scratchpad(const Arg *arg);

#endif
