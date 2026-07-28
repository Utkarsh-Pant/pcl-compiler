/*
 * TODO: this works fine for now, but ensure to add safety checks later.
 */


#include "stack.h"

struct Stack* stack_init(){

	struct Stack* new_stack = malloc(sizeof(struct Stack));
	new_stack->cap = INITIAL_STACK_SIZE;
	new_stack->top = -1;
	new_stack->arr = calloc(INITIAL_STACK_SIZE, sizeof(void*));

	return new_stack;

}
void stack_push(struct Stack* stack, void* newEle){
	if(stack->top >= stack->cap-1){
		/*Grow internal array.*/
		stack->cap = stack->cap*2;
		stack->arr = reallocarray(stack->arr, stack->cap, sizeof(void*));
	}
	stack->arr[++stack->top] = newEle;
}

void* stack_pop(struct Stack* stack){
	if(stack->top == -1) return NULL;
	void* out = stack->arr[stack->top--];
	if(stack->cap > MIN_SIZE && stack->top <= ((stack->cap)/4) -1){
		/* Shrink internal array */
		/* TODO: Might remove shrinking due to (potentionally) uneccessary cost*/
		stack->cap = stack->cap/2;
		stack->arr = reallocarray(stack->arr, stack->cap, sizeof(void*));
	}
	
	return out;

}

void* stack_peek(struct Stack* stack){
	if(stack->top == -1) return NULL;
	return stack->arr[stack->top];
}

bool stack_isEmpty(struct Stack* stack){
	return stack->top == -1;
}
