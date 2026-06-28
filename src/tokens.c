#include "tokens.h"

void printToken(struct Token* tk){

        char* dispatchTable[TOKEN_TYPE_COUNT] = {NULL};

        dispatchTable[IDENT] = "IDENT";
        dispatchTable[OP_PLUS] = "OP_PLUS";
        dispatchTable[OP_MINUS] = "OP_MINUS";
        dispatchTable[OP_DIVIDE] = "OP_DIVIDE";
        dispatchTable[OP_MULTIPLY] = "OP_MULTIPLY";
        dispatchTable[OP_NOT] = "OP_NOT";

        dispatchTable[OP_PLUS_PLUS] = "OP_PLUS_PLUS";
        dispatchTable[OP_MINUS_MINUS] = "OP_MINUS_MINUS";

        dispatchTable[OP_PLUS_EQUALS] = "OP_PLUS_EQUALS";
        dispatchTable[OP_MINUS_EQUALS] = "OP_MINUS_EQUALS";
        dispatchTable[OP_DIVIDE_EQUALS] = "OP_DIVIDE_EQUALS";
        dispatchTable[OP_MULTIPLY_EQUALS] = "OP_MULTIPLY_EQUALS";
        dispatchTable[OP_EQUALS] = "OP_EQUALS";
        dispatchTable[OP_NOT_EQUALS] = "OP_NOT_EQUALS";

        dispatchTable[OP_EQUALS_EQUALS] = "OP_EQUALS_EQUALS";
        dispatchTable[OP_LESSER_EQUALS] = "OP_LESSER_EQUALS";
        dispatchTable[OP_GREATER_EQUALS] ="OP_GREATER_EQUALS";
        dispatchTable[OP_LESSER] = "OP_LESSER";
        dispatchTable[OP_GREATER] = "OP_GREATER";
	
	dispatchTable[OP_COMMA] = "OP_COMMA";

        dispatchTable[LITERAL_INT] = "LITERAL_INT";
        dispatchTable[LITERAL_CHR] = "LITERAL_CHR";
        dispatchTable[LITERAL_STR] = "LITERAL_STR";

        dispatchTable[KWD] = "KWD";
        dispatchTable[KWD_IF] = "IF";
        dispatchTable[KWD_ELSE] = "ELSE";
        dispatchTable[KWD_INT] = "INT";
        dispatchTable[KWD_CHR] = "CHAR";
        dispatchTable[KWD_FOR] = "FOR";
        dispatchTable[KWD_WHILE] = "WHILE";
        dispatchTable[KWD_BREAK] = "BREAK";
        dispatchTable[KWD_CONTINUE] = "CONTINUE";

        for(int i = PUNC; i<TOKEN_TYPE_COUNT; i++){
                dispatchTable[i] = "PUNC_2C";
        }
        dispatchTable[PUNC] = "PUNC";
        printf("(%s)", dispatchTable[tk->type] == NULL ? "INVALID" : dispatchTable[tk->type]);
}

// this is a very temp. function that i hate.
TOKEN_TYPE getCategory(struct Token* tk){
	TOKEN_TYPE type = tk->type;
	if(type >= KWD && type <= KWD_TERM_END) return KWD;
	if(type==IDENT) return IDENT;
	if(type >= OP && type <= OP_TERM_END) return OP;
	if(type >= LITERAL && type <= LIT_TERM_END) return LITERAL;
	if(type >= PUNC && type <= PUNC_TERM_END) return PUNC;

	return ERR;	
}
