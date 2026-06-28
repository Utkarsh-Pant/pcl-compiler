#include "sym_table.h"

struct SymbolTable* init_sym_table(){
	struct SymbolTable* new_table = malloc(sizeof(struct SymbolTable));

	new_table->hmap = hmap_init();
	new_table->parent = NULL;
	new_table->current_offset = 0;


	return new_table;
}


