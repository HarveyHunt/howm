#ifndef OP_H
#define OP_H

static void op_kill(const unsigned int type, int cnt);
static void op_move_up(const unsigned int type, int cnt);
static void op_move_down(const unsigned int type, int cnt);
static void op_focus_down(const unsigned int type, int cnt);
static void op_focus_up(const unsigned int type, int cnt);
static void op_shrink_gaps(const unsigned int type, int cnt);
static void op_grow_gaps(const unsigned int type, int cnt);
static void op_cut(const unsigned int type, int cnt);

#endif
