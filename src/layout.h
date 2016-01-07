#ifndef LAYOUT_H
#define LAYOUT_H

#include "types.h"

/**
 * @file layout.h
 *
 * @author Harvey Hunt
 *
 * @date 2015
 *
 * @brief howm
 */

enum layouts { ZOOM, GRID, HSTACK, VSTACK, END_LAYOUT };

void arrange_windows(monitor_t *m);
void change_layout(monitor_t *m, const int layout);
void next_layout(monitor_t *m);
void prev_layout(monitor_t *m);
void last_layout(monitor_t *m);

#endif
