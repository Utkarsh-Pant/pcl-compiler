/**
 * @file stack.h
 * @brief Generic stack implementation
 * @ingroup helper
 *
 */
#ifndef STACK_H
#define STACK_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define INITIAL_STACK_SIZE 64
#define MIN_SIZE INITIAL_STACK_SIZE

/**
 * Generic stack structure, implemented using array of void* .
 */
struct Stack{
	/** Internal stack array. **/
	void** arr;
	/** Current stack capacity. **/
	int cap;
	/** Stack top index. **/
	int top;
};

/**
 * Initializes a stack with default parameters.
 * 
 * @return Returns heap allocated stack data structure.
 */
struct Stack* stack_init(void);

/**
 * Push an element to the stack.
 * 
 * @param stack Stack data structure
 * @param newEle Element to be pushed onto the stack.
 */
void stack_push(struct Stack* stack, void* newEle);

/**
 * Pop an element from top of the stack.
 *
 * @param stack Stack data structure
 * @return Pointer to the popped element. NULL if empty.
 */
void* stack_pop(struct Stack* stack);

/**
 * Peek top element from the stack.
 *
 * @param stack Stack data structure.
 * @return Pointer value of the top element. NULL if empty.
 */
void* stack_peek(struct Stack* stack);

/**
 * Checks if the stack is empty.
 *
 * @param stack Stack data structure.
 * @return True if the stack is empty, false otherwise.
 */
bool stack_isEmpty(struct Stack* stack);
#endif
