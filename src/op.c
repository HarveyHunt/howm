#include <string.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>

#include "client.h"
#include "helper.h"
#include "howm.h"
#include "op.h"
#include "scratchpad.h"
#include "types.h"
#include "workspace.h"

/**
 * @file op.c
 *
 * @author Harvey Hunt
 *
 * @date 2015
 *
 * @brief All of howm's operators are implemented here.
 */

static int cur_cnt = 1;

static void change_gaps(const unsigned int type, unsigned int cnt, int size);

/**
 * @brief An operator that kills an arbitrary amount of clients or workspaces.
 *
 * @param type Whether to kill workspaces or clients.
 * @param cnt How many workspaces or clients to kill.
 *
 * @ingroup operators
 */
void op_kill(const unsigned int type, unsigned int cnt)
{
	if (type == WORKSPACE) {
		log_info("Killing %d workspaces", cnt);
		while (cnt > 0) {
			kill_ws(offset_ws(mon->ws, cnt - 1));
			cnt--;
		}
	} else if (type == CLIENT) {
		log_info("Killing %d clients", cnt);
		while (cnt > 0) {
			kill_client(mon->ws, cnt == 1);
			cnt--;
		}
	}
}

/**
 * @brief Move client/s down.
 *
 * @param type We don't support moving workspaces, so this should only be
 * client.
 * @param cnt How many clients to move.
 *
 * @ingroup operators
 */
void op_move_down(const unsigned int type, unsigned int cnt)
{
	if (type == WORKSPACE)
		return;
	move_client(cnt, false);
}

/**
 * @brief Move client/s up.
 *
 * @param type We don't support moving workspaces, so this should only be
 * client.
 * @param cnt How many clients to move.
 *
 * @ingroup operators
 */
void op_move_up(const unsigned int type, unsigned int cnt)
{
	if (type == WORKSPACE)
		return;
	move_client(cnt, true);
}

/**
 * @brief Operator function to move the current focus up.
 *
 * @param type Whether to focus on clients or workspaces.
 * @param cnt The number of times to move focus.
 *
 * @ingroup operators
 */
void op_focus_up(const unsigned int type, unsigned int cnt)
{
	while (cnt > 0) {
		if (type == CLIENT)
			focus_next_client();
		else if (type == WORKSPACE)
			focus_next_ws();
		else
			return;
		cnt--;
	}
}

/**
 * @brief Operator function to move the current focus down.
 *
 * @param type Whether to focus on clients or workspaces.
 * @param cnt The number of times to move focus.
 *
 * @ingroup operators
 */
void op_focus_down(const unsigned int type, unsigned int cnt)
{
	while (cnt > 0) {
		if (type == CLIENT)
			focus_prev_client();
		else if (type == WORKSPACE)
			focus_prev_ws();
		else
			return;
		cnt--;
	}
}

/**
 * @brief An operator to grow the gaps of either workspaces or clients by
 * conf.op_gap_size.
 *
 * When the type is workspace, the gap size for that workspace is also changed.
 * This means that new windows will be spawned in with the modified gap size.
 *
 * @param type Whether the operation should be performed on a client or
 * workspace.
 * @param cnt The amount of clients or workspaces to perform the operation on.
 *
 * @ingroup operators
 */
void op_grow_gaps(const unsigned int type, unsigned int cnt)
{
	change_gaps(type, cnt, conf.op_gap_size);
}

/**
 * @brief Does the heavy lifting of changing the gaps of clients.
 *
 * @param type Whether to perform the operation on a client or workspace.
 * @param cnt The amount of times to perform the operation.
 * @param size The amount of pixels to change the gap size by. This is
 * configured through conf.op_gap_size.
 */
static void change_gaps(const unsigned int type, unsigned int cnt, int size)
{
	client_t *c = NULL;
	workspace_t *ws = NULL;

	if (type == WORKSPACE) {
		for (ws = mon->ws; ws != NULL && cnt > 0; ws = ws->next, cnt--) {
			ws->gap += size;
			log_info("Changing gaps of workspace <%d> by %dpx",
					workspace_to_index(ws), size);
			for (c = ws->head; c; c = c->next)
				change_client_gaps(c, size);
		}
	} else if (type == CLIENT) {
		c = mon->ws->c;
		while (cnt > 0) {
			log_info("Changing gaps of client <%p> by %dpx", c, size);
			change_client_gaps(c, size);
			c = next_client(c);
			cnt--;
		}
	}
}

/**
 * @brief Cut one or more clients and add them onto howm's delete register
 * stack (if there is space).
 *
 * A segment of howm's internal client list is taken and placed onto the delete
 * register stack. All clients from the list segment must be unmapped and the
 * remaining clients must be refocused.
 *
 * @param type Whether to cut an entire workspace or client.
 * @param cnt The amount of clients or workspaces to cut.
 *
 * @ingroup operators
 */
void op_cut(const unsigned int type, unsigned int cnt)
{
	client_t *tail = mon->ws->c;
	client_t *head = mon->ws->c;
	client_t *head_prev = prev_client(mon->ws->c, mon->ws);
	bool wrap = false;

	if (!head)
		return;

	if (del_reg.size >= conf.delete_register_size) {
		log_warn("No more stack space.");
		return;
	}

	if ((type == CLIENT && cnt >= mon->ws->client_cnt) || type == WORKSPACE) {
		/* TODO: Actually implement this... */
		return;

	} else if (type == CLIENT) {
		xcb_unmap_window(dpy, head->win);
		mon->ws->client_cnt--;
		while (cnt > 1) {
			if (!tail->next && next_client(tail)) {
				wrap = true;
				/* Join the list into a circular linked list,
				 * just for now so that we don't miss any
				 * clients. */
				tail->next = next_client(tail);
			}
			if (tail == mon->ws->prev_foc)
				mon->ws->prev_foc = NULL;
			tail = next_client(tail);
			xcb_unmap_window(dpy, tail->win);
			cnt--;
			mon->ws->client_cnt--;
		}

		if (head == mon->ws->head) {
			mon->ws->head = head == next_client(tail) ? NULL : next_client(tail);
		} else if (wrap) {
			mon->ws->head = tail->next;
			head_prev->next = NULL;
		} else if (tail->next != head_prev) {
			head_prev->next = wrap ? NULL : tail->next;
		}

		mon->ws->c = head_prev;
		tail->next = NULL;
		update_focused_client(head_prev);
		stack_push(&del_reg, head);
	}
}

/**
 * @brief An operator to shrink the gaps of either workspaces or clients by
 * conf.op_gap_size.
 *
 * When the type is workspace, the gap size for that workspace is also changed.
 * This means that new windows will be spawned in with the modified gap size.
 *
 * @param type Whether the operation should be performed on a client or
 * workspace.
 * @param cnt The amount of clients or workspaces to perform the operation on.
 *
 * @ingroup operators
 */
void op_shrink_gaps(const unsigned int type, unsigned int cnt)
{
	change_gaps(type, cnt, -conf.op_gap_size);
}

/**
 * @brief Set the current count for the current operator.
 *
 * @param cnt The amount of motions the operator should affect.
 *
 * @ingroup commands
 */
void count(const unsigned int cnt)
{
	if (cur_state != COUNT_STATE)
		return;
	cur_cnt = cnt;
	cur_state = MOTION_STATE;
}

/**
 * @brief Tell howm which motion is to be performed.
 *
 * This allows keybinding using an external program to still use operators.
 *
 * @param target A single char representing the motion that the operator should
 * be applied to.
 *
 * @ingroup commands
 */
void motion(char *target)
{
	int type;

	if (cur_state == OPERATOR_STATE)
		return;

	if (strncmp(target, "w", 1) == 0)
		type = WORKSPACE;
	else if (strncmp(target, "c", 1) == 0)
		type = CLIENT;
	else
		return;

	operator_func(type, cur_cnt);
	cur_state = OPERATOR_STATE;
	operator_func = NULL;
	/* Reset so that qc is equivalent to q1c. */
	cur_cnt = 1;
}
