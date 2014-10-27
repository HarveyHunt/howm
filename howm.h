#ifndef HOWM_H
#define HOWM_H

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include "config.h"

/** Calculates a mask that can be applied to a window in order to reconfigure a
 * window. */
#define MOVE_RESIZE_MASK (XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y | \
			  XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT)
/** Ensures that the number lock doesn't intefere with checking the equality
 * of two modifier masks.*/
#define CLEANMASK(mask) (mask & ~(numlockmask | XCB_MOD_MASK_LOCK))
/** Wraps up the comparison of modifier masks into a neat package. */
#define EQUALMODS(mask, omask) (CLEANMASK(mask) == CLEANMASK(omask))
/** Calculates the length of an array. */
#define LENGTH(x) (unsigned int)(sizeof(x) / sizeof(*x))
/** Checks to see if a client is floating, fullscreen or transient. */
#define FFT(c) (c->is_transient || c->is_floating || c->is_fullscreen)
/** Supresses the unused variable compiler warnings. */
#define UNUSED(x) (void)(x)
/** Determine which file descriptor is the largest and add one to it. */
#define MAX_FD(x, y) ((x) > (y) ? (x + 1) : (y + 1))


/* Add comments so that splint ignores this as it doesn't support variadic
 * macros.
 */
/*@ignore@*/
#ifdef DEBUG_ENABLE
/** Output debugging information using puts. */
#       define DEBUG(x) puts(x)
/** Output debugging information using printf to allow for formatting. */
#	define DEBUGP(M, ...) fprintf(stderr, "[DBG] %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#       define DEBUG(x) do {} while (0)
#       define DEBUGP(x, ...) do {} while (0)
#endif

#define LOG_DEBUG 1
#define LOG_INFO 2
#define LOG_WARN 3
#define LOG_ERR 4
#define LOG_NONE 5

#if LOG_LEVEL == LOG_DEBUG
#define log_debug(M, ...) fprintf(stderr, "[DEBUG] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define log_debug(x, ...) do {} while (0)
#endif


#if LOG_LEVEL <= LOG_INFO
#define log_info(M, ...) fprintf(stderr, "[INFO] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define log_info(x, ...) do {} while (0)
#endif

#if LOG_LEVEL <= LOG_WARN
#define log_warn(M, ...) fprintf(stderr, "[WARN] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define log_warn(x, ...) do {} while (0)
#endif

#if LOG_LEVEL <= LOG_ERR
#define log_err(M, ...) fprintf(stderr, "[ERROR] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define log_err(x, ...) do {} while (0)
#endif
/*@end@*/

enum states { OPERATOR_STATE, COUNT_STATE, MOTION_STATE, END_STATE };

extern int numlockmask, retval, last_ws, prev_layout, cw;
extern xcb_connection_t *dpy;
extern uint32_t border_focus, border_unfocus, border_prev_focus, border_urgent;
extern unsigned int cur_mode;
extern uint16_t screen_height, screen_width;
static bool running = true, restart;
extern int cur_state;
static xcb_screen_t *screen;
static xcb_ewmh_connection_t *ewmh;

#endif
