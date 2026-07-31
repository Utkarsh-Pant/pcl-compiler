#ifndef SEMANTIC_ANALYSER_H
#define SEMANTIC_ANALYSER_H

#include "tree.h"
#include "sym_table.h"
#include "stream.h"

int analyse(struct Stream* in, struct SymbolTable* table);

#endif
