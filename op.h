#ifndef OP_H
#define OP_H

#include <xcb/xcb.h>

enum motions { CLIENT, WORKSPACE };

/**
 * @brief Represents an operator.
 *
 * Operators perform an action upon one or more targets (identified by
 * motions).
 */
typedef struct {
	int mod; /**< The mask of the modifiers pressed. */
	xcb_keysym_t sym; /**< The keysym of the pressed key. */
	unsigned int mode; /**< The mode within which this keypress is valid. */
	void (*func)(const unsigned int type, const int cnt); /**< The function to be
								 * called when the key is pressed. */
} Operator;

/**
 * @brief Represents a motion.
 *
 * A motion can be used to target an operation at something specific- such as a
 * client or workspace.
 *
 * For example:
 *
 * q4c (Kill, 4, Clients).
 */
typedef struct {
	int mod; /**< The mask of the modifiers pressed. */
	xcb_keysym_t sym; /**< The keysym of the pressed key. */
	unsigned int type; /**< Represents whether the motion is for clients, WS etc. */
} Motion;

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
