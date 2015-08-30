#include <sys/socket.h>
#include <sys/un.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "client.h"
#include "helper.h"
#include "howm.h"
#include "ipc.h"
#include "layout.h"
#include "mode.h"
#include "op.h"
#include "scratchpad.h"
#include "types.h"
#include "workspace.h"

enum msg_type { MSG_FUNCTION = 1, MSG_CONFIG };

/**
 * @file ipc.c
 *
 * @author Harvey Hunt
 *
 * @date 2014
 *
 * @brief Everything required to parse, interpret and respond to messages that
 * are sent over IPC.
 */

static char **ipc_process_args(char *msg, int len, int *err);
static int ipc_arg_to_int(char *arg, int *err, int lower, int upper);
static int ipc_process_function(char **args);
static int ipc_process_config(char **args);
static bool ipc_arg_to_bool(char *arg, int *err);

/**
 * @brief Open a socket and return it.
 *
 * If a socket path is defined in the env variable defined as ENV_SOCK_VAR then
 * use that - else use DEF_SOCK_PATH.
 *
 * @return A socket file descriptor.
 */
int ipc_init(void)
{
	struct sockaddr_un addr;
	char *sp = NULL;
	char sock_path[256];
	int sock_fd;

	sp = getenv(ENV_SOCK_VAR);

	if (sp)
		snprintf(sock_path, sizeof(sock_path), "%s", sp);
	else
		snprintf(sock_path, sizeof(sock_path), "%s", DEF_SOCK_PATH);

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", sock_path);
	unlink(sock_path);
	sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);

	if (sock_fd == -1) {
		log_err("Couldn't create the socket.");
		exit(EXIT_FAILURE);
	}

	if (bind(sock_fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
		log_err("Couldn't bind a name to the socket.");
		exit(EXIT_FAILURE);
	}

	if (listen(sock_fd, 1) == -1) {
		log_err("Listening error.");
		exit(EXIT_FAILURE);
	}

	return sock_fd;
}

/**
 * @brief Delete the UNIX socket file.
 */
void ipc_cleanup(void)
{
	char *sp = getenv(ENV_SOCK_VAR);

	if (sp)
		unlink(sp);
	else
		unlink(DEF_SOCK_PATH);
}

/**
 * @brief Process a message depending on its type - a config message or a
 * function call message.
 *
 * @param msg A buffer containing the message sent by cottage.
 * @param len The length of the message.
 *
 * @return An error code resulting from processing msg.
 */
int ipc_process(char *msg, int len)
{
	int err = IPC_ERR_NONE;
	char **args = ipc_process_args(msg, len, &err);

	if (**args == MSG_FUNCTION)
		err = ipc_process_function(args + 1);
	else if (**args == MSG_CONFIG)
		err = ipc_process_config(args + 1);
	else
		err = IPC_ERR_UNKNOWN_TYPE;

	free(args);
	return err;
}

/**
 * @brief Receive a char array from a UNIX socket and subsequently call a
 * function, passing the args from within msg.
 *
 * @param args The args (as strings).
 *
 * @return The error code, as set by this function itself or those that it
 * calls.
 */
static int ipc_process_function(char **args)
{
	int err = IPC_ERR_NONE;
	int i = 0;

#define CALL_INT(func, arg, lower, upper) \
	do { \
		i = ipc_arg_to_int(arg, &err, lower, upper); \
		if (err == IPC_ERR_NONE) \
			func(i); \
	} while (0)

	if (strncmp(*args, "teleport_client", strlen("teleport_client")) == 0) {
		CALL_INT(teleport_client, *(args + 1), TOP_LEFT, BOTTOM_RIGHT);
	} else if (strncmp(*args, "quit_howm", strlen("quit_howm")) == 0) {
		CALL_INT(quit_howm, *(args + 1), EXIT_SUCCESS, EXIT_FAILURE);
	} else if (strncmp(*args, "current_to_ws", strlen("current_to_ws")) == 0) {
		CALL_INT(current_to_ws, *(args + 1), 1, WORKSPACES);
	} else if (strncmp(*args, "resize_float_width", strlen("resize_float_width")) == 0) {
		CALL_INT(resize_float_width, *(args + 1), -100, 100);
	} else if (strncmp(*args, "resize_float_height", strlen("resize_float_height")) == 0) {
		CALL_INT(resize_float_height, *(args + 1), -100, 100);
	} else if (strncmp(*args, "move_float_x", strlen("move_float_x")) == 0) {
		CALL_INT(move_float_x, *(args + 1), -100, 100);
	} else if (strncmp(*args, "move_float_y", strlen("move_float_y")) == 0) {
		CALL_INT(move_float_y, *(args + 1), -100, 100);
	} else if (strncmp(*args, "resize_master", strlen("resize_master")) == 0) {
		CALL_INT(resize_master, *(args + 1), -100, 100);
	} else if (strncmp(*args, "change_ws", strlen("change_ws")) == 0) {
		CALL_INT(change_ws, *(args + 1), 1, WORKSPACES);
	} else if (strncmp(*args, "change_mode", strlen("change_mode")) == 0) {
		CALL_INT(change_mode, *(args + 1), NORMAL, END_MODES - 1);
	} else if (strncmp(*args, "change_layout", strlen("change_layout")) == 0) {
		CALL_INT(change_layout, *(args + 1), ZOOM, END_LAYOUT - 1);
	} else if (strncmp(*args, "count", strlen("count")) == 0) {
		CALL_INT(count, *(args + 1), 1, 9);
#undef CALL_INT
	} else if (strncmp(*args, "move_current_down", strlen("move_current_down")) == 0) {
		move_current_down();
	} else if (strncmp(*args, "move_current_up", strlen("move_current_up")) == 0) {
		move_current_up();
	} else if (strncmp(*args, "focus_next_client", strlen("focus_next_client")) == 0) {
		focus_next_client();
	} else if (strncmp(*args, "focus_prev_client", strlen("focus_prev_client")) == 0) {
		focus_prev_client();
	} else if (strncmp(*args, "toggle_float", strlen("toggle_float")) == 0) {
		toggle_float();
	} else if (strncmp(*args, "toggle_fullscreen", strlen("toggle_fullscreen")) == 0) {
		toggle_fullscreen();
	} else if (strncmp(*args, "focus_urgent", strlen("focus_urgent")) == 0) {
		focus_urgent();
	} else if (strncmp(*args, "send_to_scratchpad", strlen("send_to_scratchpad")) == 0) {
		send_to_scratchpad();
	} else if (strncmp(*args, "get_from_scratchpad", strlen("get_from_scratchpad")) == 0) {
		get_from_scratchpad();
	} else if (strncmp(*args, "make_master", strlen("make_master")) == 0) {
		make_master();
	} else if (strncmp(*args, "toggle_bar", strlen("toggle_bar")) == 0) {
		toggle_bar();
	} else if (strncmp(*args, "focus_next_ws", strlen("focus_next_ws")) == 0) {
		focus_next_ws();
	} else if (strncmp(*args, "focus_prev_ws", strlen("focus_prev_ws")) == 0) {
		focus_prev_ws();
	} else if (strncmp(*args, "focus_last_ws", strlen("focus_last_ws")) == 0) {
		focus_last_ws();
	} else if (strncmp(*args, "paste", strlen("paste")) == 0) {
		paste();
	} else if (strncmp(*args, "next_layout", strlen("next_layout")) == 0) {
		next_layout();
	} else if (strncmp(*args, "prev_layout", strlen("prev_layout")) == 0) {
		prev_layout();
	} else if (strncmp(*args, "last_layout", strlen("last_layout")) == 0) {
		last_layout();
	} else if (strncmp(*args, "spawn", strlen("spawn")) == 0) {
		spawn(args + 1);
	} else if (strncmp(*args, "motion", strlen("motion")) == 0) {
		motion(*(args + 1));
	} else if (strncmp(*args, "op_kill", strlen("op_kill")) == 0) {
		operator_func = op_kill;
		cur_state = COUNT_STATE;
	} else if (strncmp(*args, "op_move_up", strlen("op_move_up")) == 0) {
		operator_func = op_move_up;
		cur_state = COUNT_STATE;
	} else if (strncmp(*args, "op_move_down", strlen("op_move_down")) == 0) {
		operator_func = op_move_down;
		cur_state = COUNT_STATE;
	} else if (strncmp(*args, "op_focus_down", strlen("op_focus_down")) == 0) {
		operator_func = op_focus_down;
		cur_state = COUNT_STATE;
	} else if (strncmp(*args, "op_focus_up", strlen("op_focus_up")) == 0) {
		operator_func = op_focus_up;
		cur_state = COUNT_STATE;
	} else if (strncmp(*args, "op_shrink_gaps", strlen("op_shrink_gaps")) == 0) {
		operator_func = op_shrink_gaps;
		cur_state = COUNT_STATE;
	} else if (strncmp(*args, "op_grow_gaps", strlen("op_grow_gaps")) == 0) {
		operator_func = op_grow_gaps;
		cur_state = COUNT_STATE;
	} else if (strncmp(*args, "op_cut", strlen("op_cut")) == 0) {
		operator_func = op_cut;
		cur_state = COUNT_STATE;
	} else {
		return IPC_ERR_NO_FUNC;
	}

	return err;
}

/**
 * @brief Convert a numerical string into a decimal value, such as "12"
 * becoming 12.
 *
 * Minus signs are handled. It is assumed that a two digit number won't start
 * with a zero. Args with more than two digits will not be accepted, nor will
 * args that aren't numerical.
 *
 * @param arg The string to be converted.
 * @param err Where errors are reported.
 * @param lower The lower bound for the returned value. Note: This is inclusive
 * @param upper The upper bound for the returned value. Note: This is inclusive
 *
 * @return The decimal representation of arg.
 */
static int ipc_arg_to_int(char *arg, int *err, int lower, int upper)
{
	int ret = 0;

	if (!arg) {
		*err = IPC_ERR_TOO_FEW_ARGS;
		return ret;
	} else {
		ret = atoi(arg);
	}

	if (ret > upper)
		*err = IPC_ERR_ARG_TOO_LARGE;
	else if (ret < lower)
		*err = IPC_ERR_ARG_TOO_SMALL;
	return ret;
}

/**
 * @brief Accepts a char array and convert it into an array of strings.
 *
 * msg is split into strings (delimited by a null character) and placed in an
 * array. err is set with a corresponding error (such as args too few args), or
 * nothing.
 *
 * XXX: args must be freed by the caller.
 *
 * @param msg A char array that is read from a UNIX socket.
 * @param len The length of data in msg.
 * @param err Where any errors will be stored.
 *
 * @return A pointer to an array of strings, each one representing an argument
 * that has been passed over a UNIX socket.
 */
static char **ipc_process_args(char *msg, int len, int *err)
{
	int argc = 0, i = 0, arg_start = 0, lim = 2;
	char **args = malloc(lim * sizeof(char *));

	if (!args) {
		*err = IPC_ERR_ALLOC;
		return NULL;
	}

	for (; i < len; i++) {
		if (msg[i] == 0) {
			*(args + argc++) = msg + arg_start;
			arg_start = i + 1;

			if (argc == lim) {
				lim *= 2;
				char **new = realloc(args, lim * sizeof(char *));

				if (!new) {
					*err = IPC_ERR_ALLOC;
					free(args);
					return NULL;
				}
				args = new;
			}
		}
	}

	/* Make room to add the NULL after the last character. */
	if (argc == lim) {
		char **new = realloc(args, (lim + 1) * sizeof(char *));

		if (!new) {
			*err = IPC_ERR_ALLOC;
			return NULL;
		}
		args = new;
	}

	/* The end of the array should be NULL, as the whole array can be passed to
	 * spawn() and that expects a NULL terminated array.
	 *
	 * Use argc here as args are zero indexed. */
	*(args + argc) = NULL;

	if (argc < 1) {
		*err = IPC_ERR_TOO_FEW_ARGS;
		free(args);
		return NULL;
	}

	return args;
}

/**
 * @brief Process a config message. If the config option isn't recognised, set
 * err to IPC_ERR_NO_CONFIG.
 *
 * @param args An array of strings representing the args.
 *
 * @return err containing the error (or lack of) that has occurred.
 */
static int ipc_process_config(char **args)
{
	int err = IPC_ERR_NONE;
	int i = 0;
	bool b = false;

	if (!args[0] || !args[1])
		return IPC_ERR_TOO_FEW_ARGS;

#define SET_INT(opt, arg, lower, upper) \
	do { \
		i = ipc_arg_to_int(arg, &err, lower, upper); \
			if (err == IPC_ERR_NONE) \
				opt = i; \
	} while (0)

	if (strcmp("border_px", *args) == 0)
		SET_INT(conf.border_px, *(args + 1), 0, 32);
	else if (strcmp("float_spawn_height", *args) == 0)
		SET_INT(conf.float_spawn_height, *(args + 1), 1, screen_height);
	else if (strcmp("float_spawn_width", *args) == 0)
		SET_INT(conf.float_spawn_width, *(args + 1), 1, screen_width);
	else if (strcmp("scratchpad_height", *args) == 0)
		SET_INT(conf.scratchpad_height, *(args + 1), 1, screen_height);
	else if (strcmp("scratchpad_width", *args) == 0)
		SET_INT(conf.scratchpad_width, *(args + 1), 1, screen_width);
	else if (strcmp("op_gap_size", *args) == 0)
		SET_INT(conf.op_gap_size, *(args + 1), 0, 32);
	else if (strcmp("bar_height", *args) == 0)
		SET_INT(conf.bar_height, *(args + 1), 0, screen_height);
#undef SET_INT
#define SET_BOOL(opt, arg) \
	do { \
		b = ipc_arg_to_bool(arg, &err); \
			if (err == IPC_ERR_NONE) \
				opt = b; \
	} while (0)

	else if (strcmp("focus_mouse", *args) == 0)
		SET_BOOL(conf.focus_mouse, *(args + 1));
	else if (strcmp("focus_mouse_click", *args) == 0)
		SET_BOOL(conf.focus_mouse_click, *(args + 1));
	else if (strcmp("follow_move", *args) == 0)
		SET_BOOL(conf.follow_move, *(args + 1));
	else if (strcmp("zoom_gap", *args) == 0)
		SET_BOOL(conf.zoom_gap, *(args + 1));
	else if (strcmp("center_floating", *args) == 0)
		SET_BOOL(conf.center_floating, *(args + 1));
	else if (strcmp("bar_bottom", *args) == 0)
		SET_BOOL(conf.bar_bottom, *(args + 1));
#undef SET_BOOL
#define SET_COLOUR(opt, arg) \
	do { \
		if (strlen(arg) > 7) \
			return IPC_ERR_ARG_TOO_LARGE; \
		else if (strlen(arg) < 7) \
			return IPC_ERR_ARG_TOO_SMALL; \
		opt = get_colour(arg); \
	} while (0)

	else if (strcmp("border_focus", *args) == 0)
		SET_COLOUR(conf.border_focus, *(args + 1));
	else if (strcmp("border_unfocus", *args) == 0)
		SET_COLOUR(conf.border_unfocus, *(args + 1));
	else if (strcmp("border_prev_focus", *args) == 0)
		SET_COLOUR(conf.border_prev_focus, *(args + 1));
	else if (strcmp("border_urgent", *args) == 0)
		SET_COLOUR(conf.border_urgent, *(args + 1));
	else
		err = IPC_ERR_NO_CONFIG;
	update_focused_client(wss[cw].current);
	return err;
#undef SET_COLOUR
}

/**
 * @brief Convert an argument to a boolean.
 *
 * t and 1 are considered true, f and 0 are considered false.
 *
 * @param arg A string containing the argument.
 * @param err Where an error code should be stored.
 *
 * @return A boolean, depending on whether the argument was true or false.
 */
static bool ipc_arg_to_bool(char *arg, int *err)
{
	if (strcmp("true", arg) == 0
			|| strcmp("t", arg) == 0
			|| strcmp("1", arg) == 0) {
		return true;
	} else if (strcmp("false", arg) == 0
			|| strcmp("f", arg) == 0
			|| strcmp("0", arg) == 0) {
		return false;
	} else {
		*err = IPC_ERR_ARG_NOT_BOOL;
		return false;
	}
}
