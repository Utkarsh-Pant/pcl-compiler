#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "stream.h"

#ifndef LEXER_H
#define LEXER_H

#define INITIAL_STRING_SIZE 64

// #define LEXER_DEBUG

#define UPD_SUC EXIT_SUCCESS
#define UPD_FIN 242 
//242 because not : )
#define UPD_ERR EXIT_FAILURE

#define COMMENT_CHR '#'
#define CHKN_WS(x) (x!='\n' && x!= '\t' && x!= ' ')
#define CHK_WS(x) (x=='\n' || x=='\t' || x==' ')
typedef enum{
	ERR, // ERR ( = 0 )
	WS,
	IDENT, // Identifier
        KWD, // Keyword (I have decided not to be verbose here.
	     // I believe this decision should aid the parser slightly.	
	OP, // Ooperator. 
	OP_PLUS, // + NOTE: for now +,- are only binary!
	OP_MINUS, // -
	OP_DIVIDE, // /
	OP_MULTIPLY, // *
	OP_NOT,	// !
	
	OP_PLUS_PLUS, // ++
	OP_MINUS_MINUS, // --
	
	OP_PLUS_EQUALS, // +=
	OP_MINUS_EQUALS, // -=
	OP_DIVIDE_EQUALS, // /=
	OP_MULTIPLY_EQUALS, // *=
	OP_EQUALS, // =

	OP_NOT_EQUALS, // !=
	OP_EQUALS_EQUALS, // ==
	OP_LESSER_EQUALS, // <=
	OP_GREATER_EQUALS, // >=
	OP_LESSER, // <
	OP_GREATER, // >

	OP_COMMA, // I hate this, but for now yep :_)

	LITERAL, // Literals.
	LITERAL_INT, // 242
	LITERAL_CHR, // 'a'
	LITERAL_STR, // "hello reader : )"

	PUNC, // For now punctuator is kept as single but may change.

	TOKEN_TYPE_COUNT // MUST BE AT END. DO NOT CHANGE.
} TOKEN_TYPE;

typedef enum{
	KWD_IF,
	KWD_ELSE,
	KWD_INT,
	KWD_CHR,
	KWD_FOR,
	KWD_WHILE,	
	KWD_BREAK,
	KWD_CONTINUE
} KEYWORD;

struct Token{

	TOKEN_TYPE type;
	union{
		int i;
		char c;
		char* s;
	} value;
	size_t cap; // To manage allocated sizes.

};


int lex(char[static 1], struct Stream*);
void printToken(struct Token*);
#endif
