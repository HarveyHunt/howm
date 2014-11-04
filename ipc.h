#ifndef IPC_H
#define IPC_H

/**
 * @file ipc.h
 *
 * @author Harvey Hunt
 *
 * @date 2014
 *
 * @brief howm
 */

enum ipc_errs { IPC_ERR_NONE, IPC_ERR_SYNTAX, IPC_ERR_ALLOC, IPC_ERR_NO_CMD, IPC_ERR_TOO_MANY_ARGS,
	IPC_ERR_TOO_FEW_ARGS, IPC_ERR_ARG_NOT_INT, IPC_ERR_ARG_TOO_LARGE, IPC_ERR_UNKNOWN_TYPE };
enum arg_types { TYPE_IGNORE, TYPE_INT, TYPE_CMD };

int ipc_init(void);
int ipc_process(char *msg, int len);

static Command commands[] = {
	{"resize_master", resize_master, NULL, 1, TYPE_INT},
	{"change_layout", change_layout, NULL, 1, TYPE_INT},
	{"next_layout", next_layout, NULL, 0, TYPE_INT},
	{"previous_layout", previous_layout, NULL, 0, TYPE_INT},
	{"last_layout", last_layout, NULL, 0, TYPE_INT},
	{"change_mode", change_mode, NULL, 1, TYPE_INT},
	{"toggle_float", toggle_float, NULL, 0, TYPE_INT},
	{"toggle_fullscreen", toggle_fullscreen, NULL, 0, TYPE_INT},
	{"quit_howm", quit_howm, NULL, 1, TYPE_INT},
	{"restart_howm", restart_howm, NULL, 1, TYPE_INT},
	{"resize_master", resize_master, NULL, 1, TYPE_INT},
	{"toggle_bar", toggle_bar, NULL, 0, TYPE_INT},
	{"replay", replay, NULL, 0, TYPE_INT},
	{"paste", paste, NULL, 0, TYPE_INT},
	{"send_to_scratchpad", send_to_scratchpad, NULL, 0, TYPE_INT},
	{"get_from_scratchpad", get_from_scratchpad, NULL, 0, TYPE_INT},
	{"resize_float_height", resize_float_height, NULL, 1, TYPE_INT},
	{"resize_float_width", resize_float_width, NULL, 1, TYPE_INT},
	{"move_float_x", move_float_x, NULL, 1, TYPE_INT},
	{"move_float_y", move_float_y, NULL, 1, TYPE_INT},
	{"teleport_client", teleport_client, NULL, 1, TYPE_INT},
	{"focus_urgent", focus_urgent, NULL, 0, TYPE_INT},
	{"focus_prev_client", focus_prev_client, NULL, 0, TYPE_INT},
	{"focus_next_client", focus_next_client, NULL, 0, TYPE_INT},
	{"move_current_up", move_current_up, NULL, 0, TYPE_INT},
	{"move_current_down", move_current_down, NULL, 0, TYPE_INT},
	{"focus_last_ws", focus_last_ws, NULL, 0, TYPE_INT},
	{"focus_next_ws", focus_next_ws, NULL, 0, TYPE_INT},
	{"focus_prev_ws", focus_prev_ws, NULL, 0, TYPE_INT},
	{"make_master", make_master, NULL, 0, TYPE_INT},
	{"change_ws", change_ws, NULL, 1, TYPE_INT},
	{"current_to_ws", current_to_ws, NULL, 1, TYPE_INT},
	{"spawn", spawn, NULL, 1, TYPE_CMD},

	{"op_kill", NULL, op_kill, 2, TYPE_IGNORE},
	{"op_move_up", NULL, op_move_up, 2, TYPE_IGNORE},
	{"op_move_down", NULL, op_move_down, 2, TYPE_IGNORE},
	{"op_shrink_gaps", NULL, op_shrink_gaps, 2, TYPE_IGNORE},
	{"op_grow_gaps", NULL, op_grow_gaps, 2, TYPE_IGNORE},
	{"op_cut", NULL, op_cut, 2, TYPE_IGNORE},
	{"op_focus_down", NULL, op_focus_down, 2, TYPE_IGNORE},
	{"op_focus_up", NULL, op_focus_up, 2, TYPE_IGNORE}
};

#endif
