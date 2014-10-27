#ifndef MISC_H
#define MISC_H
static void apply_rules(Client *c);
static void howm_info(void);
static void save_last_ocm(void (*op) (const unsigned int, int), const unsigned int type, int cnt);
static void save_last_cmd(void (*cmd)(const Arg *), const Arg *arg);
static void replay(const Arg *arg);
static void paste(const Arg *arg);
static int get_non_tff_count(void);
static Client *get_first_non_tff(void);
static uint32_t get_colour(char *colour);
static void spawn(const Arg *arg);
static void setup(void);
static void move_client(int cnt, bool up);
static void focus_window(xcb_window_t win);
static void quit_howm(const Arg *arg);
static void restart_howm(const Arg *arg);
static void cleanup(void);
static void delete_win(xcb_window_t win);
static void setup_ewmh(void);

#endif
