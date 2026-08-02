#ifndef TREE_H
#define TREE_H

#include "tokens.h"


struct TableRecord;
struct SymbolTable;

extern int ast_node_count;
extern struct Stream* global_string_stream;
extern size_t global_string_stream_count;

struct AST{

	TOKEN_TYPE type;
	union token_val value;
	size_t children_count;
	struct AST** children;

	// Semantic Attributes:
	struct TableRecord* entry;
	struct SymbolTable* table;
	TOKEN_TYPE result_type;
	
};

void printAST(struct AST*);
void addChild(struct AST*, struct AST*);
struct AST* createAST(struct Token*);


#endif
