#include "parser.h"

static void printDFS(struct AST* root){
	if(root==NULL) return;
}
void printTree(struct AST* root){
	printf("----Printing Tree----\n");
	printf("---------------------\n");
}

static PREC precidence_table[TOKEN_TYPE_COUNT]={
	[OP_PLUS_PLUS] = PREC_UNARY,
	[OP_MINUS_MINUS] = PREC_UNARY,
	[OP_NOT] = PREC_UNARY,

	[OP_MULTIPLY] = PREC_MULT,
	[OP_DIVIDE] = PREC_MULT,

	[OP_PLUS] = PREC_ADD,
	[OP_MINUS] = PREC_ADD,

	[OP_LESSER] = PREC_REL,
	[OP_GREATER] = PREC_REL,
	[OP_LESSER_EQUALS] = PREC_REL,
	[OP_GREATER_EQUALS] = PREC_REL,

	[OP_NOT_EQUALS] = PREC_EQ,
	[OP_EQUALS_EQUALS] = PREC_EQ,

	[OP_PLUS_EQUALS] = PREC_ASS,
	[OP_MINUS_EQUALS] = PREC_ASS,
	[OP_DIVIDE_EQUALS] = PREC_ASS,
	[OP_MULTIPLY_EQUALS] = PREC_ASS,
	[OP_EQUALS] = PREC_ASS,

	[OP_COMMA] = PREC_COMMA

};

static int parse_expr(struct Token** tokens, struct AST* out){
	if(tokens==NULL) return PARSE_ERR;
	struct Generic_Stack* node_stk = createStack();
	struct Generic_Stack* op_stk = createStack();

	for(int i = 0; tokens[i] != NULL; i++){

	}
	return PARSE_ERR;

}

static int parse_stmt(struct Token** tokens, struct AST* out){

}

void parse(struct Stream* input, struct Stream* output){

	/*
	 * Current Plan:
	 * If first is kwd -> statement, otherwise expression.
	 * For statement, send it to parse_stmt and let it process.
	 *
	 * Otherwise read till ';' and then send it to parse_expr
	*/



}
