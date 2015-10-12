#ifndef HANDLER_H
#define HANDLER_H

#include <xcb/xcb.h>

/**
 * @file handler.h
 *
 * @author Harvey Hunt
 *
 * @date 2015
 *
 * @brief howm
 */

void handle_event(xcb_generic_event_t *ev);

#endif
