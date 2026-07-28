#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "stack.h"
#include "lexer.h"
#include "tree.h"
#include "tokens.h"

#ifndef PARSER_H
#define PARSER_H

#define PARSE_ERR EXIT_FAILURE
#define PARSE_SUCC EXIT_SUCCESS


typedef enum{
	ASS_LEFT,
	ASS_RIGHT
} ASS;

typedef enum{
	#define PREC(label) label,
	#define MAP(a,b,c)
	#define ST(a)
	#include "parser_mappings.def"
	#undef PREC
	#undef MAP
	#undef ST

} PREC;

typedef enum{

	STMT,
	EXPR,
	VAL

} NODE_TYPE;


int parse(struct Stream*, struct Stream*);


#endif 
