#include "tree.h"

static char* dispatchTable[TOKEN_TYPE_COUNT] = {
	#define X(name, desc) [name] = desc,
	#define UN(a,b)
	#include "tokens.def"
	#undef X
	#undef UN
};

static void printDFSsexy(struct AST* node, int count){

	if(node == NULL) return;

	for(int i = 0; i<count; i++)printf("-");
	printf(">");
	switch(node->type){
		case LITERAL_INT: printf("%s[%d]", dispatchTable[node->type], node->value.i);
				  break;
		case LITERAL_CHR: printf("%s[%c]", dispatchTable[node->type], node->value.c);
				  break;
		case LITERAL_STR: printf("%s[%s]", dispatchTable[node->type], node->value.s);
				  break;
		case IDENT: printf("%s[%s]", dispatchTable[node->type], node->value.s);
			    break;
		default: printf("%s", dispatchTable[node->type]); 
	}

	printf("\n");
	for(int i = 0; i< node->children_count; i++){
		printDFSsexy(node->children[i], count+1);
	}

}

void printAST(struct AST* node){
	printDFSsexy(node, 0);
}

void addChild(struct AST* parent, struct AST* child){
	if(child==NULL) return;

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
	temp->value = token->value;

	if( 
		token->type == LITERAL_STR ||
		token->type == IDENT
	)
		token->value.s = NULL;	

	return temp;
}
