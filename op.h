#ifndef OP_H
#define OP_H

#include <xcb/xcb.h>

enum motions { CLIENT, WORKSPACE };

void op_kill(const unsigned int type, int cnt);
void op_move_up(const unsigned int type, int cnt);
void op_move_down(const unsigned int type, int cnt);
void op_focus_down(const unsigned int type, int cnt);
void op_focus_up(const unsigned int type, int cnt);
void op_shrink_gaps(const unsigned int type, int cnt);
void op_grow_gaps(const unsigned int type, int cnt);
void op_cut(const unsigned int type, int cnt);
void change_gaps(const unsigned int type, int cnt, int size);

#endif
