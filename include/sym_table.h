
#ifndef SYM_TABLE_H
#define SYM_TABLE_H

#include <stdbool.h>
#include "hmap.h"
#include "tokens.h"
#include "stream.h"

#define SYM_SUCC 0
#define SYM_ERR 1

struct AST;

struct VarData{
	int stack_offset;
	bool assigned;
};

struct FuncData{
	size_t arg_count;
	bool is_extern;
};

struct TableRecord{
	char* name;
	TOKEN_TYPE type;
	bool is_func;

	union{
		struct VarData v;
		struct FuncData f;
	} data;
};

struct SymbolTable{

	struct HashMap* hmap; // Contains TableRecord Entries
	struct SymbolTable* parent;

	size_t total_bytes;
	struct Stream* keys; // For printing only, can be removed
};

struct SymbolTable* symtable_init(struct SymbolTable* parent);
struct TableRecord* symtable_get(struct SymbolTable* table, struct AST* node);
struct TableRecord* symtable_get_local(struct SymbolTable* table, struct AST* node);
struct TableRecord* symtable_add(struct SymbolTable* table, struct AST* node, TOKEN_TYPE type);
void symtable_print(struct SymbolTable* table);
#endif
