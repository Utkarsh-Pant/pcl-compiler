#ifndef ASM_GEN_H
#define ASM_GEN_H

#include "tac.h"
#include "sym_table.h"

void gen_asm(struct TACList* list, struct SymbolTable* table, const char* filename);

#endif
