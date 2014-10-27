#ifndef HOWM_H
#define HOWM_H

#include "config.h"

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

#endif
