#ifndef LAYOUT_H
#define LAYOUT_H

enum layouts { ZOOM, GRID, HSTACK, VSTACK, END_LAYOUT };

static void stack(void);
static void grid(void);
static void zoom(void);
void arrange_windows(void);

#endif
