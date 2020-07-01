#ifndef OSLAB02_STACK_H_
#define OSLAB02_STACK_H_

#define STACKLIM 256

struct stack;

struct stack *stack_create();
void stack_free(struct stack **stack);

int stack_is_empty(struct stack *stack);

int stack_push(struct stack *stack, char *item);
const char *stack_pop(struct stack *stack);

#endif // OSLAB02_STACK_H_
