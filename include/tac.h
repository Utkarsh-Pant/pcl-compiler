#ifndef TAC_H
#define TAC_H

#include "sym_table.h"
#include "tokens.h"

typedef enum{

	#define TAC(node) node,
	#include "tac.def"

} TAC_TYPE;

struct TACArg{
	bool is_literal;
	union{
		struct TableRecord* arg;
		union token_val val;
	};

};

struct TAC{
	struct TACArg arg1;
	struct TACArg arg2;

	struct TableRecord* res;

	struct TAC* next;
	struct TAC* prev;


	TAC_TYPE op; // Tac operation to be performed
};

#endif
