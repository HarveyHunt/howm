#ifndef HANDLER_H
#define HANDLER_H

void enter_event(xcb_generic_event_t *ev);
void destroy_event(xcb_generic_event_t *ev);
void button_press_event(xcb_generic_event_t *ev);
void key_press_event(xcb_generic_event_t *ev);
void map_event(xcb_generic_event_t *ev);
void configure_event(xcb_generic_event_t *ev);
void unmap_event(xcb_generic_event_t *ev);
void client_message_event(xcb_generic_event_t *ev);
void handle_event(xcb_generic_event_t *ev);

#endif
