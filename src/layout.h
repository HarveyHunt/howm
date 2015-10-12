#ifndef LAYOUT_H
#define LAYOUT_H

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

void arrange_windows(void);
void change_layout(const int layout);
void next_layout(void);
void prev_layout(void);
void last_layout(void);

#endif
