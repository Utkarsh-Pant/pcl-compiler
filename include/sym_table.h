
#ifndef SYM_TABLE_H
#define SYM_TABLE_H

#include "hmap.h"

struct TableRecord{
	char* name;
	size_t type; // Might change to enum.
	size_t size;
	int address;
};

struct SymbolTable{
	struct HashMap* hmap; // Contains TableRecord Entries
	int current_offset;
	struct SymbolTable* parent;
};

#endif
