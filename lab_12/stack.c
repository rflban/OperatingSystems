#include "stack.h"

#include <stdlib.h>

struct stack
{
    char *data[STACKLIM];
    int size;
};

struct stack *stack_create()
{
    struct stack *stack = malloc(sizeof(struct stack));
    stack->size = 0;

    return stack;
}

int stack_is_empty(struct stack *stack)
{
    return (stack->size == 0);
}

int stack_push(struct stack *stack, char *element)
{
    if (stack->size >= STACKLIM)
        return -1;

    stack->data[stack->size++] = element;

    return 0;
}

const char *stack_pop(struct stack *stack)
{
    return stack->data[--stack->size];
}

void stack_free(struct stack **stack)
{
    if (stack != NULL && *stack != NULL)
        free(*stack);

    *stack = NULL;
}
