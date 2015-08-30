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

enum ipc_errs { IPC_ERR_NONE, IPC_ERR_SYNTAX, IPC_ERR_ALLOC, IPC_ERR_NO_FUNC,
	IPC_ERR_TOO_MANY_ARGS, IPC_ERR_TOO_FEW_ARGS, IPC_ERR_ARG_NOT_INT,
	IPC_ERR_ARG_NOT_BOOL, IPC_ERR_ARG_TOO_LARGE, IPC_ERR_ARG_TOO_SMALL,
	IPC_ERR_UNKNOWN_TYPE, IPC_ERR_NO_CONFIG };
enum arg_types { TYPE_IGNORE, TYPE_INT, TYPE_STR };

void ipc_cleanup(void);
int ipc_init(void);
int ipc_process(char *msg, int len);

#endif
