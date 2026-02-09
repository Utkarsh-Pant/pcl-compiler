#include <stdio.h>
#include <stdlib.h>
#include "stack.h"
#include "lexer.h"

#ifndef PARSER_H
#define PARSER_H

#define PARSE_ERR EXIT_FAILURE
#define PARSE_SUCC EXIT_SUCCESS

typedef enum{
	PREC_ERR, // (=0)
	PREC_UNARY, // Unary Operators
	PREC_MULT, // Mulitplicative
	PREC_ADD, // Additive
	PREC_REL, // Relational
	PREC_EQ, // Equality
	PREC_ASS, // Assignment
	PREC_COMMA // Comma
} PREC;

typedef enum{

	STMT,
	EXPR,
	VAL

} NODE_TYPE;

struct AST{

	NODE_TYPE type;
	union{
		int i;
		char c;
		char* s;
	} value;
	size_t children_count;
	struct AST* children;

};

void parse(struct Stream*, struct Stream*);


#endif 
