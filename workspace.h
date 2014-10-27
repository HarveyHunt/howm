#ifndef WORKSPACE_H
#define WORKSPACE_H

static void kill_ws(const int ws);
static void toggle_bar(const Arg *arg);
static void resize_master(const Arg *arg);
static void focus_next_ws(const Arg *arg);
static void focus_prev_ws(const Arg *arg);
static void focus_last_ws(const Arg *arg);
static void change_ws(const Arg *arg);
static int correct_ws(int ws);

#endif

