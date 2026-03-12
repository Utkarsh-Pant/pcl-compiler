#include <stdio.h>
#include <stdlib.h>

#ifndef TOKENS_H
#define TOKENS_H

typedef enum{
        ERR, // ERR ( = 0 )
        WS,
        IDENT, // Identifier
        KWD, // Keyword (I have decided not to be verbose here.
             // I believe this decision should aid the parser slightly.
             // Update: 15/02/2026 I was wrong. Very wrong and doomb.
        KWD_IF,
        KWD_ELSE,
        KWD_INT,
        KWD_CHR,
        KWD_FOR,
        KWD_WHILE,
        KWD_BREAK,
        KWD_CONTINUE,
	KWD_TERM_END,

        OP, // Ooperator.
        OP_PLUS, // + NOTE: for now +,- are only binary!
        OP_MINUS, // -
        OP_DIVIDE, // /
        OP_MULTIPLY, // *
        OP_NOT, // !

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

	OP_TERM_END,

        LITERAL, // Literals.
        LITERAL_INT, // 242
        LITERAL_CHR, // 'a'
        LITERAL_STR, // "hello reader : )"
	LIT_TERM_END,

        PUNC, // For now punctuator is kept as single but may change.
        PUNC_OPEN_PAR, // (
        PUNC_CLOSE_PAR, // )
        PUNC_OPEN_BRACKET, // [
        PUNC_CLOSE_BRACKET, // ]
        PUNC_OPEN_BRACE, // {
        PUNC_CLOSE_BRACE, // }
        PUNC_COMMA, // ,
        PUNC_SC, // ;
	PUNC_TERM_END,


        TOKEN_TYPE_COUNT // MUST BE AT END. DO NOT CHANGE.
} TOKEN_TYPE;

// TODO: REMOVE
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

void printToken(struct Token*);
TOKEN_TYPE getCategory(struct Token*);
#endif
