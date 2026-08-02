#ifndef TAC_H
#define TAC_H

#include <stdbool.h>
#include "sym_table.h"
#include "tokens.h"
#include "tree.h"

#define TAC_SUCC
#define TAC_ERR

typedef enum{

	#define TAC(node) node,
	#include "tac.def"

} TAC_TYPE;

struct TACArg{
	bool is_literal;
	union{
		struct TableRecord* arg;
		union token_val value;
	} data;

};

struct TAC{
	struct TACArg arg1;
	struct TACArg arg2;

	struct TableRecord* res;

	int tac_id;
	TAC_TYPE op; // Tac operation to be performed
};

struct TACList{
	struct TAC* arr;
	size_t n;
	size_t cap;
};

void gen_tac(struct Stream* astStream, struct TACList* tac, struct SymbolTable* table);
void TAC_print(struct TAC* tac);
struct TACList* TACList_init(void);

#endif
