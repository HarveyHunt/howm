#include <sys/socket.h>
#include <sys/un.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "command.h"
#include "op.h"
#include "ipc.h"
#include "helper.h"
#include "howm.h"
#include "config.h"

#define SET_INT(opt, arg, lower, upper) \
	i = ipc_arg_to_int(arg, &err, lower, upper); \
		if (err == IPC_ERR_NONE) \
			opt = i;

#define SET_BOOL(opt, arg) \
	b = ipc_arg_to_bool(arg, &err); \
		if (err == IPC_ERR_NONE) \
			opt = b;

#define SET_COLOUR(opt, arg) \
	if (strlen(arg) > 7) \
		return IPC_ERR_ARG_TOO_LARGE; \
	else if (strlen(arg) < 7) \
		return IPC_ERR_ARG_TOO_SMALL; \
	opt = get_colour(arg);

enum msg_type { MSG_FUNCTION = 1, MSG_CONFIG };
static int cur_cnt = 1;

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

int ipc_init(void)
{
	struct sockaddr_un addr;
	int sock_fd;

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", SOCK_PATH);
	unlink(SOCK_PATH);
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
 * @param msg A char array from the UNIX socket. In the form:
 *
 * COMMAND\0ARG1\0ARG2\0 ....
 *
 * @param len The length of the msg.
 *
 * @return The error code, as set by this function itself or those that it
 * calls.
 */
static int ipc_process_function(char **args)
{
	unsigned int i;
	bool found = false;
	int err = IPC_ERR_NONE;

	for (i = 0; i < LENGTH(commands); i++)
		if (strcmp(*args, commands[i].name) == 0) {
			found = true;
			if (commands[i].argc == 0) {
				commands[i].func(&(Arg){ NULL });
				break;
			} else if (commands[i].argc == 1 && *(args + 1) && commands[i].arg_type == TYPE_INT) {
				Arg a = { .i = ipc_arg_to_int(*(args + 1), &err, -100, 100) };
				commands[i].func(&a);
				break;
			} else if (commands[i].argc == 1 && *(args + 1) && commands[i].arg_type == TYPE_STR) {
				Arg a = { .cmd = args + 1 };
				commands[i].func(&a);
				break;
			} else if (commands[i].arg_type == TYPE_IGNORE) {
				operator_func = commands[i].operator;
				cur_state = COUNT_STATE;
			} else {
				err = IPC_ERR_SYNTAX;
			}
		}
	err = found == true ? err : IPC_ERR_NO_FUNC;
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

static int ipc_process_config(char **args)
{
	int err = IPC_ERR_NONE;
	int i = 0;
	bool b = false;

	if (!(args + 1))
		return IPC_ERR_TOO_FEW_ARGS;
	if (strcmp("border_px", *args) == 0) {
		SET_INT(conf.border_px, *(args + 1), 0, 32);
	} else if (strcmp("float_spawn_height", *args) == 0) {
		SET_INT(conf.float_spawn_height, *(args + 1), 1, screen_height);
	} else if (strcmp("float_spawn_width", *args) == 0) {
		SET_INT(conf.float_spawn_width, *(args + 1), 1, screen_width);
	} else if (strcmp("scratchpad_height", *args) == 0) {
		SET_INT(conf.scratchpad_height, *(args + 1), 1, screen_height);
	} else if (strcmp("scratchpad_width", *args) == 0) {
		SET_INT(conf.scratchpad_width, *(args + 1), 1, screen_width);
	} else if (strcmp("op_gap_size", *args) == 0) {
		SET_INT(conf.op_gap_size, *(args + 1), 0, 32);
	} else if (strcmp("bar_height", *args) == 0) {
		SET_INT(conf.bar_height, *(args + 1), 0, screen_height);
	} else if (strcmp("focus_mouse", *args) == 0) {
		SET_BOOL(conf.focus_mouse, *(args + 1));
	} else if (strcmp("focus_mouse_click", *args) == 0) {
		SET_BOOL(conf.focus_mouse_click, *(args + 1));
	} else if (strcmp("follow_move", *args) == 0) {
		SET_BOOL(conf.follow_move, *(args + 1));
	} else if (strcmp("zoom_gap", *args) == 0) {
		SET_BOOL(conf.zoom_gap, *(args + 1));
	} else if (strcmp("center_floating", *args) == 0) {
		SET_BOOL(conf.center_floating, *(args + 1));
	} else if (strcmp("bar_bottom", *args) == 0) {
		SET_BOOL(conf.bar_bottom, *(args + 1));
	} else if (strcmp("border_focus", *args) == 0) {
		SET_COLOUR(conf.border_focus, *(args + 1));
	} else if (strcmp("border_unfocus", *args) == 0) {
		SET_COLOUR(conf.border_unfocus, *(args + 1));
	} else if (strcmp("border_prev_focus", *args) == 0) {
		SET_COLOUR(conf.border_prev_focus, *(args + 1));
	} else if (strcmp("border_urgent", *args) == 0) {
		SET_COLOUR(conf.border_urgent, *(args + 1));
	}
	update_focused_client(wss[cw].current);
	return err;
}

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
