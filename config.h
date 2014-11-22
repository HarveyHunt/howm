#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include <stdlib.h>
#include <X11/keysym.h>
#include <X11/X.h>
#include "op.h"
#include "command.h"
#include "layout.h"
#include "client.h"
#include "workspace.h"

/**
 * @file config.h
 *
 * @author Harvey Hunt
 *
 * @date 2014
 *
 * @brief howm
 */

/*
 *┌────────────┐
 *│╻ ╻┏━┓╻ ╻┏┳┓│
 *│┣━┫┃ ┃┃╻┃┃┃┃│
 *│╹ ╹┗━┛┗┻┛╹ ╹│
 *└────────────┘
 */

/** Enable debugging output */
#define DEBUG_ENABLE false
/** How much detail should be logged. A LOG_LEVEL of INFO will log almost
 * everything, LOG_WARN will log warnings and errors and LOG_ERR will log only
 * errors.
 *
 * LOG_NONE means nothing will be logged.
 *
 * LOG_DEBUG should be used by developers.
 */
#define LOG_LEVEL LOG_DEBUG


/* Rules that are applied to clients as they are spawned. */
static const Rule rules[] = {
	/* Class, WS, follow, float, fullscreen */
	{"dwb", 3, false, false, false},
	{"mpv", 5, false, false, false}
};
#endif
