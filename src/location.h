#ifndef LOCATION_H
#define LOCATION_H

#include <xcb/xcb.h>

#include "types.h"

/**
 * @file location.h
 *
 * @author Harvey Hunt
 *
 * @date 2016
 *
 * @brief howm
 */

bool loc_win(location_t *loc, xcb_window_t w);
bool loc_client(location_t *loc, client_t *c);

#endif
