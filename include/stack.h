#include <stdio.h>
#include <stdlib.h>

#ifndef STACK_H
#define STACK_H

#define INITIAL_STACK_SIZE 64
#define MIN_SIZE INITIAL_STACK_SIZE


// Stack of void pointers.
struct Generic_Stack{
	void** arr;
	int cap;
	int top;
};

struct Generic_Stack* createStack(void);

void push(struct Generic_Stack* stack, void*);
void* pop(struct Generic_Stack* stack);
void* peek(struct Generic_Stack* stack);
int isEmptyStack(struct Generic_Stack*);
#endif
