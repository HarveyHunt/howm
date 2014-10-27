#ifndef MISC_H
#define MISC_H
static void replay(const Arg *arg);
static void paste(const Arg *arg);
static int get_non_tff_count(void);
static Client *get_first_non_tff(void);
static uint32_t get_colour(char *colour);
static void spawn(const Arg *arg);
static void setup(void);
static void move_client(int cnt, bool up);
static void quit_howm(const Arg *arg);
static void restart_howm(const Arg *arg);
static void cleanup(void);

#endif
