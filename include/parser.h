#include <stdio.h>
#include <stdlib.h>
#include "stack.h"
#include "lexer.h"
#include "tree.h"

#ifndef PARSER_H
#define PARSER_H

#define PARSE_ERR EXIT_FAILURE
#define PARSE_SUCC EXIT_SUCCESS

typedef enum{
	PREC_ERR, // (=0)
	PREC_COMMA,
	PREC_ASS,
	PREC_EQ,
	PREC_REL,
	PREC_ADD,
	PREC_MULT,
	PREC_UNARY
} PREC;

typedef enum{

	STMT,
	EXPR,
	VAL

} NODE_TYPE;


int parse(struct Stream*, struct Stream*);


#endif 
