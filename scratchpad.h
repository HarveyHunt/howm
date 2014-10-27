#ifndef SCRATCHPAD_H
#define SCRATCHPAD_H

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
	int size; /**< The amount of items in the stack. */
	Client **contents; /**< The contents is an array of linked lists. Storage
			is malloced later as we don't know the size yet.*/
};


static void stack_push(struct stack *s, Client *c);
static Client *stack_pop(struct stack *s);
static void stack_init(struct stack *s);
static void stack_free(struct stack *s);

#endif
