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
static int ipc_arg_to_int(char *arg, int *err);
static int ipc_process_function(char **args);
static int ipc_process_config(char **args);

int ipc_init(void)
{
	struct sockaddr_un addr;
	int sock_fd;

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", conf.sock_path);
	unlink(conf.sock_path);
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
				commands[i].func(&(Arg){ .i = ipc_arg_to_int(*(args + 1), &err) });
				break;
			} else if (commands[i].argc == 1 && *(args + 1) && commands[i].arg_type == TYPE_STR) {
				commands[i].func(&(Arg){ .cmd = args + 1 });
				break;
			} else if (commands[i].argc == 2 && *(args + 1) && *(args + 2) && **(args + 2) == 'w') {
				commands[i].operator(WORKSPACE, ipc_arg_to_int(*(args + 1), &err));
				break;
			} else if (commands[i].argc == 2 && *(args + 1) && *(args + 2) && **(args + 2) == 'c') {
				commands[i].operator(CLIENT, ipc_arg_to_int(*(args + 1), &err));
				break;
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
static int ipc_arg_to_int(char *arg, int *err)
{
	int sign = 1;

	if (arg[0] == '-') {
		sign = -1;
		arg++;
	}

	if (strlen(arg) == 1 && '0' < *arg && *arg <= '9') {
		if ('0' <= *arg && *arg <= '9')
			return sign * (*arg - '0');
		*err = IPC_ERR_ARG_NOT_INT;
		return 0;
	} else if (strlen(arg) == 2 && '0' < arg[0] && arg[0] <= '9'
			&& '0' <= arg[1] && arg[1] <= '9') {
		if ('0' < arg[0] && arg[0] <= '9' && '0' <= arg[1] && arg[1] <= '9')
			return sign * (10 * (arg[0] - '0') + (arg[1] - '0'));
		*err = IPC_ERR_ARG_NOT_INT;
		return 0;
	} else {
		*err = IPC_ERR_ARG_TOO_LARGE;
		return 0;
	}
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
	if (!(args + 1))
		return IPC_ERR_TOO_FEW_ARGS;
	if (strcmp("border_px", *args) == 0) {
		conf.border_px = ipc_arg_to_int(*(args + 1), &err);
	} else if(strcmp("gap", *args) == 0) {
		conf.gap = ipc_arg_to_int(*(args + 1), &err);
	}
	return err;
}
