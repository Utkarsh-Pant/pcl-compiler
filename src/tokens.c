#include "tokens.h"

static char* dispatchTable[TOKEN_TYPE_COUNT] = {
    	#define X(name, desc) [name] = desc,
	#define UN(a,b)
    	#include "tokens.def"
    	#undef X
	#undef UN
};

static TOKEN_TYPE unaryMap[TOKEN_TYPE_COUNT] = {
	#define X(name, desc) [name] = name,
	#define UN(name, un_name) [name] = un_name,
	#include "tokens.def"
	#undef X
	#undef UN
};

void printToken(struct Token* tk){

	switch(tk->type){
		case LITERAL_INT: printf("%s[%d]", dispatchTable[tk->type], tk->value.i);
				  break;
		case LITERAL_CHR: printf("%s[%c]", dispatchTable[tk->type], tk->value.c);
				  break;
		case LITERAL_STR: printf("%s[%s]", dispatchTable[tk->type], tk->value.s);
				  break;
		case IDENT: printf("%s[%s]", dispatchTable[tk->type], tk->value.s);
			    break;
		default: printf(" %s ", dispatchTable[tk->type]);
	}

}

TOKEN_TYPE getCategory(TOKEN_TYPE type){
	if(type >= KWD && type <= KWD_TERM_END) return KWD;
	if(type >= OP && type <= OP_TERM_END) return OP;
	if(type >= LITERAL && type <= LIT_TERM_END) return LITERAL;
	if(type >= PUNC && type <= PUNC_TERM_END) return PUNC;

	return type;	
}

bool is_unary(struct Token* tk){
	TOKEN_TYPE type = tk->type;
	if(type>OP_UN && type<OP_UN_TERM_END) return true;
	return false;
}

void demote_unary(struct Token* tk){
	tk->type = unaryMap[tk->type];	
}
