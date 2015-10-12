#ifndef OP_H
#define OP_H

/**
 * @file op.h
 *
 * @author Harvey Hunt
 *
 * @date 2015
 *
 * @brief howm
 */

enum motions { CLIENT, WORKSPACE };

void (*operator_func)(const unsigned int type, unsigned int cnt);

void op_kill(const unsigned int type, unsigned int cnt);
void op_move_up(const unsigned int type, unsigned int cnt);
void op_move_down(const unsigned int type, unsigned int cnt);
void op_focus_down(const unsigned int type, unsigned int cnt);
void op_focus_up(const unsigned int type, unsigned int cnt);
void op_shrink_gaps(const unsigned int type, unsigned int cnt);
void op_grow_gaps(const unsigned int type, unsigned int cnt);
void op_cut(const unsigned int type, unsigned int cnt);
void count(const unsigned int cnt);
void motion(char *target);

#endif
