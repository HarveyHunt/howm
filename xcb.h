#ifndef XCB_H
#define XCB_H
static void grab_keys(void);
static xcb_keycode_t *keysym_to_keycode(xcb_keysym_t sym);
static void grab_keycode(xcb_keycode_t *keycode, const int mod);
static void elevate_window(xcb_window_t win);
static void move_resize(xcb_window_t win, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
static void set_border_width(xcb_window_t win, uint16_t w);
static void get_atoms(char **names, xcb_atom_t *atoms);
static void check_other_wm(void);
static xcb_keysym_t keycode_to_keysym(xcb_keycode_t keycode);
static void grab_buttons(Client *c);
static void ewmh_process_wm_state(Client *c, xcb_atom_t a, int action);

#endif
