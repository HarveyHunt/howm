#ifndef LAYOUT_H
#define LAYOUT_H

enum layouts { ZOOM, GRID, HSTACK, VSTACK, END_LAYOUT };

void stack(void);
void grid(void);
void zoom(void);
void arrange_windows(void);

#endif
