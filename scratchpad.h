#ifndef SCRATCHPAD_H
#define SCRATCHPAD_H
static void stack_push(struct stack *s, Client *c);
static Client *stack_pop(struct stack *s);
static void stack_init(struct stack *s);
static void stack_free(struct stack *s);

#endif
