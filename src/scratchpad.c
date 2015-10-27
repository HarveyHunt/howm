#include <stdlib.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>

#include "scratchpad.h"
#include "client.h"
#include "helper.h"
#include "howm.h"

/**
 * @file scratchpad.c
 *
 * @author Harvey Hunt
 *
 * @date 2015
 *
 * @brief The stack implementation and appropriate functions required for
 * sending clients (or groups of clients) to the scratchpad.
 */

struct stack del_reg;
static client_t *scratchpad;

/**
 * @brief Dynamically allocate space for the contents of the stack.
 *
 * We don't know how big the stack will be when the struct is defined, so we
 * need to allocate it dynamically.
 *
 * @param s The stack that needs to have its contents allocated.
 */
void stack_init(struct stack *s)
{
	s->contents = (client_t **)malloc(sizeof(client_t) * conf.delete_register_size);
	if (!s->contents) {
		log_err("Failed to allocate memory for stack.");
		exit(EXIT_FAILURE);
	}
}

/**
 * @brief Free the allocated contents.
 *
 * @param s The stack that needs to have its contents freed.
 */
void stack_free(struct stack *s)
{
	free(s->contents);
	s->contents = NULL;
}

/**
 * @brief Pushes a client onto the stack, as long as it isn't full.
 *
 * @param s The stack.
 * @param c The client to be pushed on. This client is treated as the head of a
 * linked list.
 */
void stack_push(struct stack *s, client_t *c)
{
	if (!c || !s) {
		return;
	} else if (s->size >= conf.delete_register_size) {
		log_warn("Can't push <%p> onto stack <%p>- it is full", c, s);
		return;
	}
	s->contents[++(s->size)] = c;
}

/**
 * @brief Remove the top item from the stack and return it.
 *
 * @param s The stack to be popped from.
 *
 * @return The client that was at the top of the stack. It acts as the head of
 * the linked list of clients.
 */
client_t *stack_pop(struct stack *s)
{
	if (!s) {
		return NULL;
	} else if (s->size == 0) {
		log_warn("Can't pop from stack <%p> as it is empty.", s);
		return NULL;
	}
	return s->contents[(s->size)--];
}

/**
 * @brief Send a client to the scratchpad and unmap it.
 *
 * @ingroup commands
 */
void send_to_scratchpad(void)
{
	client_t *c = wss[cw].current;

	if (scratchpad || !c)
		return;

	log_info("Sending client <%p> to scratchpad", c);
	if (prev_client(c, cw))
		prev_client(c, cw)->next = c->next;

	/* TODO: This should be in a reusable function. */
	if (c == wss[cw].prev_foc)
		wss[cw].prev_foc = prev_client(wss[cw].current, cw);
	if (c == wss[cw].current || !wss[cw].head->next)
		wss[cw].current = wss[cw].prev_foc ? wss[cw].prev_foc : wss[cw].head;
	if (c == wss[cw].head) {
		wss[cw].head = c->next;
		wss[cw].current = c->next;
	}

	xcb_unmap_window(dpy, c->win);
	wss[cw].client_cnt--;
	update_focused_client(wss[cw].current);
	scratchpad = c;
}

/**
 * @brief Get a client from the scratchpad, attach it as the last item in the
 * client list and set it to float.
 *
 * @ingroup commands
 */
void get_from_scratchpad(void)
{
	if (!scratchpad)
		return;
	/* TODO: This should be in a reusable function. */
	if (!wss[cw].head)
		wss[cw].head = scratchpad;
	else if (!wss[cw].head->next)
		wss[cw].head->next = scratchpad;
	else
		prev_client(wss[cw].head, cw)->next = scratchpad;

	wss[cw].prev_foc = wss[cw].current;
	wss[cw].current = scratchpad;

	scratchpad = NULL;
	wss[cw].client_cnt++;

	wss[cw].current->is_floating = true;
	wss[cw].current->rect.width = conf.scratchpad_width;
	wss[cw].current->rect.height = conf.scratchpad_height;
	wss[cw].current->rect.x = (screen_width / 2) - (wss[cw].current->rect.width / 2);
	wss[cw].current->rect.y = (screen_height - wss[cw].bar_height - wss[cw].current->rect.height) / 2;

	xcb_map_window(dpy, wss[cw].current->win);
	update_focused_client(wss[cw].current);
}
