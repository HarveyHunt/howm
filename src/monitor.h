#ifndef MONITOR_H
#define MONITOR_H

#include <xcb/xproto.h>

#include "types.h"
/**
 * @file monitor.h
 *
 * @author Harvey Hunt
 *
 * @date 2015
 *
 * @brief howm
 */

void scan_monitors(void);
uint32_t monitor_to_index(const monitor_t *m);
monitor_t *index_to_monitor(uint32_t index);
void focus_monitor(monitor_t *m);
void remove_monitor(monitor_t *m);
monitor_t *point_to_monitor(xcb_point_t point);

#endif
