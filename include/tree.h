#include "tokens.h"

#ifndef TREE_H
#define TREE_H


struct AST{

	TOKEN_TYPE type;
	union token_val value;
	size_t children_count;
	struct AST** children;

};

void printAST(struct AST*);
void addChild(struct AST*, struct AST*);
struct AST* createAST(struct Token*);


#endif
