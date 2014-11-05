#include <xcb/xcb_ewmh.h>
#include "workspace.h"
#include "howm.h"
#include "helper.h"

/**
 * @file workspace.c
 *
 * @author Harvey Hunt
 *
 * @date 2014
 *
 * @brief Helper functions for dealing with an entire workspace or being able
 * to correctly calculate a workspace index.
 */

/**
 * @brief Kills the given workspace.
 *
 * @param ws The workspace to be killed.
 */
void kill_ws(const int ws)
{
	log_info("Killing off workspace <%d>", ws);
	while (wss[ws].head)
		kill_client(ws, wss[ws].client_cnt == 1
				&& cw == ws);
}

/**
 * @brief Correctly wrap a workspace number.
 *
 * This prevents workspace numbers from being greater than conf.workspaces or less
 * than 1.
 *
 * @param ws The value that needs to be corrected.
 *
 * @return A corrected workspace number.
 */
int correct_ws(int ws)
{
	if (ws > conf.workspaces)
		return ws - conf.workspaces;
	if (ws < 1)
		return ws + conf.workspaces;

	return ws;
}

