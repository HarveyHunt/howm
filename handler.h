#ifndef HANDLER_H
#define HANDLER_H
static void enter_event(xcb_generic_event_t *ev);
static void destroy_event(xcb_generic_event_t *ev);
static void button_press_event(xcb_generic_event_t *ev);
static void key_press_event(xcb_generic_event_t *ev);
static void map_event(xcb_generic_event_t *ev);
static void configure_event(xcb_generic_event_t *ev);
static void unmap_event(xcb_generic_event_t *ev);
static void client_message_event(xcb_generic_event_t *ev);
static void handle_event(xcb_generic_event_t *ev);

#endif
