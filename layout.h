#ifndef LAYOUT_H
#define LAYOUT_H

enum layouts { ZOOM, GRID, HSTACK, VSTACK, END_LAYOUT };

static void change_layout(const Arg *arg);
static void next_layout(const Arg *arg);
static void previous_layout(const Arg *arg);
static void last_layout(const Arg *arg);
static void stack(void);
static void grid(void);
static void zoom(void);
void arrange_windows(void);

static void(*layout_handler[]) (void) = {
	[GRID] = grid,
	[ZOOM] = zoom,
	[HSTACK] = stack,
	[VSTACK] = stack
};

#endif
