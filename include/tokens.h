/**
 * @file tokens.h
 * @brief Contains token definitions and related utilities.
 * @ingroup core
 * 
 */

#ifndef TOKENS_H
#define TOKENS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/**
 * Defines the token-set for PCL
 * The following tokens are not directly emitted by the lexer, but rather temporarily promoted by the parser:
 * OP_UN_PLUS
 * OP_UN_MINUS
 * PRE/POST TOKENS
 *  
 */
typedef enum{
        #define X(name, desc) name, 
	#include "tokens.def"
        TOKEN_TYPE_COUNT
} TOKEN_TYPE;

/**
 * Used to store literal token values, and identifiers.
 */
union token_val{
	int i;
	char c;
	char* s;
};

struct Token{

        TOKEN_TYPE type;
        union token_val value;
        size_t cap; // To manage allocated sizes.

};

void printToken(struct Token* tok);
TOKEN_TYPE getCategory(TOKEN_TYPE type);
bool is_unary(TOKEN_TYPE type);
void demote_unary(struct Token* tok);
#endif
