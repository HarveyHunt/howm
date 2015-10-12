#ifndef SCRATCHPAD_H
#define SCRATCHPAD_H

#include "types.h"

/**
 * @file scratchpad.h
 *
 * @author Harvey Hunt
 *
 * @date 2015
 *
 * @brief howm
 */

/**
 * @brief Represents a stack. This stack is going to hold linked lists of
 * clients. An example of the stack is below:
 *
 * TOP
 * ==========
 * c1->c2->c3->NULL
 * ==========
 * c1->NULL
 * ==========
 * c1->c2->c3->NULL
 * ==========
 * BOTTOM
 *
 */
struct stack {
	unsigned int size; /**< The amount of items in the stack. */
	Client **contents; /**< The contents is an array of linked lists. Storage
			is malloced later as we don't know the size yet.*/
};

extern struct stack del_reg;

void stack_push(struct stack *s, Client *c);
Client *stack_pop(struct stack *s);
void stack_init(struct stack *s);
void stack_free(struct stack *s);
void send_to_scratchpad(void);
void get_from_scratchpad(void);

#endif
