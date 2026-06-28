#include "tree.h"

static void printDFSsexy(struct AST* node, int count){

	if(node == NULL) return;

	for(int i = 0; i<count; i++)printf("-");
	printf(">");
	char* dispatchTable[TOKEN_TYPE_COUNT] = {NULL};

	dispatchTable[BLOCK] = "BLOCK";

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
        printf("%s\n", dispatchTable[node->type] == NULL ? "INVALID" : dispatchTable[node->type]);

	for(int i = 0; i< node->children_count; i++){
		printDFSsexy(node->children[i], count+1);
	}

}

void printAST(struct AST* node){
	printDFSsexy(node, 0);
}

void addChild(struct AST* parent, struct AST* child){
	parent->children = realloc(parent->children, (parent->children_count+1)*sizeof(struct AST*));
	(parent->children)[parent->children_count] = child;
	parent->children_count++;
}

struct AST* createAST(struct Token* token){
	struct AST* temp = malloc(sizeof(struct AST));
	if (token == NULL){
		temp->type = ERR;
		temp->children_count = 0;
		temp->children = NULL;
		return temp;
	}

	temp->type = token->type;
	temp->children_count = 0;
	temp->children = NULL;
	temp->val = token->value;

	if( 
		token->type == LITERAL_STR ||
		token->type == IDENT
	)
		token->value.s = NULL;	

	return temp;
}
