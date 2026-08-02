#include "tac.h"

static struct TACArg generate(struct AST* node, struct TACList* lst, struct SymbolTable* table);

static int label_count = 0;
static int tvar_count = 0;

static TAC_TYPE TACMap[TOKEN_TYPE_COUNT] = {
	#define TAC_TOKEN_MAP(op, tac) [op] = tac,
	#include "tac.def"
};

static char* TACPrint[TOKEN_TYPE_COUNT] = {
	#define TAC(tac) [tac] = #tac,
	#include "tac.def"
};

static int get_labelID(){
	label_count++;
	return label_count;
}

static int get_tempID(){
	tvar_count++;
	return tvar_count;
}
struct TACArg gen_arg(struct TableRecord* arg){
	struct TACArg new = (struct TACArg){0}; 
	new.is_literal = false;
	new.data.arg = arg;

	return new;
}

struct TACArg gen_literal_arg(union token_val value){

	struct TACArg new = (struct TACArg){0}; 
	new.is_literal = true;
	new.data.value = value;

	return new;
}

struct TableRecord* gen_temp_var(TOKEN_TYPE type, struct SymbolTable* table){
	//Create dummy node for injection
	struct AST* dummy = createAST(NULL);
	dummy->type = IDENT;
	dummy->value.s = calloc(sizeof(char), 8);
	sprintf(dummy->value.s, "_t%d", get_tempID());
	struct TableRecord* res = symtable_add(table, dummy, type);

	//yay memmory leak :)
	return res;
}

struct TACList* TACList_init(){
	struct TACList* new = (struct TACList*) malloc(sizeof(struct TACList));
	new->arr  = (struct TAC*) calloc(sizeof(struct TAC), 2*ast_node_count);	
	new->n = 0;
	new->cap = 2*ast_node_count;

	return new;
}

void TACList_push(struct TACList* lst, struct TACArg arg1, struct TACArg arg2, struct TableRecord* res, int id, TAC_TYPE op){
	if(lst->n >= lst->cap){
		lst->cap = 2*lst->cap;
		lst->arr = reallocarray(
				lst->arr,
				lst->cap,
				sizeof(struct TAC)
				);
	}
	lst->n++;
	lst->arr[lst->n-1].arg1 = arg1;
	lst->arr[lst->n-1].arg2 = arg2;
	lst->arr[lst->n-1].res = res;
	lst->arr[lst->n-1].tac_id = id; //Used for labels and temporary vars
	lst->arr[lst->n-1].op = op;

}

void TAC_print(struct TAC* tac){
    const char* arg1_str = "-";
    if (tac->arg1.is_literal) {
        arg1_str = "LITERAL";
    } else if (tac->arg1.data.arg != NULL) {
        arg1_str = tac->arg1.data.arg->name;
    }

    const char* arg2_str = "-";
    if (tac->arg2.is_literal) {
        arg2_str = "LITERAL";
    } else if (tac->arg2.data.arg != NULL) {
        arg2_str = tac->arg2.data.arg->name;
    }

    const char* res_str = tac->res ? tac->res->name : "-";

    printf("%s < %s, %s, %s > [ID: %d]\n",
            TACPrint[tac->op],
            arg1_str,
            arg2_str,
            res_str,
            tac->tac_id);
}


static struct TACArg generate_KWD_IF(struct AST* node, struct TACList* lst, struct SymbolTable* table){
	//Evaluate condition.
	struct TACArg cond = generate(node->children[0], lst, table);

	//Check for condition using JMF (jump if false)
	int label_end = get_labelID();
	TACList_push(
		lst,
		cond,
		(struct TACArg){0},
		NULL,
		label_end,
		TAC_JMF
	);
	//Generate body tac
	generate(node->children[1], lst, table);
	
	int label_exit = -1;
	if(node->children_count>2){
		label_exit = get_labelID();

		TACList_push(
			lst,
			(struct TACArg){0},
			(struct TACArg){0},
			NULL,
			label_exit,
			TAC_JMP
		);
	};

	//Generate end/else label
	TACList_push(
		lst,
		(struct TACArg){0},
		(struct TACArg){0},
		NULL,
		label_end, //label ID
		TAC_LABEL
	);

	//Generate else
	if(label_exit!=-1) {
		generate(node->children[2], lst, table);
		TACList_push(
			lst,
			(struct TACArg){0},
			(struct TACArg){0},
			NULL,
			label_exit, //label ID
			TAC_LABEL
		);
	}
	return (struct TACArg){0};
}

static struct TACArg generate_KWD_WHILE(struct AST* node, struct TACList* lst, struct SymbolTable* table){
	int label_start = get_labelID();

	//Generate initial label
	TACList_push(
		lst,
		(struct TACArg){0},
		(struct TACArg){0},
		NULL,
		label_start, //label ID
		TAC_LABEL
	);

	//Generate condition check
	struct TACArg cond = generate(node->children[0], lst, table);

	//Generate Condition-based skip
	int label_end = get_labelID();
	TACList_push(
		lst,
		cond,
		(struct TACArg){0},
		NULL,
		label_end,
		TAC_JMF
	);

	//Generate body
	generate(node->children[1], lst, table);

	//Go back to start
	TACList_push(
		lst,
		(struct TACArg){0},
		(struct TACArg){0},
		NULL,
		label_start,
		TAC_JMP
	);

	//End label
	TACList_push(
		lst,
		(struct TACArg){0},
		(struct TACArg){0},
		NULL,
		label_end, //label ID
		TAC_LABEL
	);

	return (struct TACArg){0};
}

static struct TACArg generate_KWD_DECL(struct AST* node, struct TACList* lst, struct SymbolTable* table){
	for(int i = 0; i<node->children_count; i++){
		struct AST* child_node = node->children[i];
		if(child_node->children_count > 0){
		//Generate copy label
			struct TACArg val = generate(child_node->children[0], lst, table);

			TACList_push(
				lst,
				val,
				(struct TACArg){0},
				child_node->entry,
				0,
				TAC_COPY	
			);
		}
	}


	return (struct TACArg){0};
}


static struct TACArg generate_KWD(struct AST* node, struct TACList* lst, struct SymbolTable* table){
	switch(node->type){
		#define TAC_KWD(kwd) case kwd: return generate_##kwd(node, lst, table);
		#define TAC_DTYPE(kwd) case kwd: return generate_KWD_DECL(node, lst, table);
		#include "tac.def"
		default: return (struct TACArg){0};
	}
	return (struct TACArg){0};
}

static struct TACArg generate_IDENT(struct AST* node, struct TACList* lst, struct SymbolTable* table){
	return gen_arg(node->entry);
}

static struct TACArg generate_LITERAL(struct AST* node, struct TACList* lst, struct SymbolTable* table){
	return gen_literal_arg(node->value);
}

static struct TACArg generate_OP_DEFAULT(struct AST* node, struct TACList* lst, struct SymbolTable* table){

	// Process Children
	struct TACArg arg1 = generate(node->children[0], lst, table);
	struct TACArg arg2 = node->children_count > 1? generate(node->children[1], lst, table) : (struct TACArg){0}; 

	struct TableRecord* res = gen_temp_var(node->result_type, table);

	TACList_push(
			lst,
			arg1,
			arg2,
			res,
			0,
			TACMap[node->type]	
		);

	return gen_arg(res);
	
}

static struct TACArg generate_OP_EQUALS(struct AST* node, struct TACList* lst, struct SymbolTable* table){
	struct TACArg arg = generate(node->children[1], lst, table);
	TACList_push(
		lst,
		arg,
		(struct TACArg){0},
		node->children[0]->entry,
		0,
		TAC_COPY	
		);

	return gen_arg(node->children[0]->entry);
}

static struct TACArg generate_OP_PLUS_PLUS(struct AST* node, struct TACList* lst, struct SymbolTable* table){
	struct TACArg arg1 = gen_arg(node->children[0]->entry);
	union token_val value;
	value.i = 1;
	struct TACArg arg2 = gen_literal_arg(value);

	TACList_push(
			lst,
			arg1,
			arg2,
			node->children[0]->entry,
			0,
			TAC_ADD	
		);

	return arg1;
}

static struct TACArg generate_OP_MINUS_MINUS(struct AST* node, struct TACList* lst, struct SymbolTable* table){

	struct TACArg arg1 = gen_arg(node->children[0]->entry);
	union token_val value;
	value.i = 1;
	struct TACArg arg2 = gen_literal_arg(value);

	TACList_push(
			lst,
			arg1,
			arg2,
			node->children[0]->entry,
			0,
			TAC_SUB
		);

	return arg1;

}
static struct TACArg generate_OP_PLUS_PLUS_POST(struct AST* node, struct TACList* lst, struct SymbolTable* table){

	struct TACArg arg1 = gen_arg(node->children[0]->entry);
	union token_val value;
	value.i = 1;
	
	struct TACArg arg2 = gen_literal_arg(value);

	struct TableRecord* res = gen_temp_var(node->result_type, table);

	TACList_push(
			lst,
			arg1,
			(struct TACArg){0},
			res,
			0,
			TAC_COPY
		);

	TACList_push(
			lst,
			arg1,
			arg2,
			node->children[0]->entry,
			0,
			TAC_ADD	
		);

	return gen_arg(res);

}
static struct TACArg generate_OP_MINUS_MINUS_POST(struct AST* node, struct TACList* lst, struct SymbolTable* table){

	struct TACArg arg1 = gen_arg(node->children[0]->entry);
	union token_val value;
	value.i = 1;
	
	struct TACArg arg2 = gen_literal_arg(value);

	struct TableRecord* res = gen_temp_var(node->result_type, table);

	TACList_push(
			lst,
			arg1,
			(struct TACArg){0},
			res,
			0,
			TAC_COPY
		);

	TACList_push(
			lst,
			arg1,
			arg2,
			node->children[0]->entry,
			0,
			TAC_SUB
		);

	return gen_arg(res);

}
static struct TACArg generate_OP_COMPOUND(struct AST* node, struct TACList* lst, struct SymbolTable* table, TOKEN_TYPE op_first){
	
	struct TACArg arg1 = gen_arg(node->children[0]->entry);
	struct TACArg arg2 = generate(node->children[1], lst, table);

	TACList_push(
		lst,
		arg1,
		arg2,
		node->children[0]->entry,
		0,
		TACMap[op_first]		
	);	

	return arg1;

}


static struct TACArg generate_OP(struct AST* node, struct TACList* lst, struct SymbolTable* table){

	switch(node->type){
		#define OP_MAP(op) case op: return generate_##op(node, lst, table);
		#define OP_MAP_COMPOUND(op_org, op_first) case op_org: return generate_OP_COMPOUND(node, lst, table, op_first);
		#include "tac.def"
		default: return generate_OP_DEFAULT(node, lst, table);
	}

}

static struct TACArg generate_CONV(struct AST* node, struct TACList* lst, struct SymbolTable* table){
	struct TACArg arg = generate(node->children[0], lst, table);
	struct TableRecord* res = gen_temp_var(node->result_type, table);
	TACList_push(
		lst,
		arg,
		(struct TACArg){0},
		res,
		0,
		TAC_CAST
		);
	return gen_arg(res);

}


static struct TACArg generate(struct AST* node, struct TACList* lst, struct SymbolTable* table){
	
	if(node==NULL) return (struct TACArg){0};
	
	TOKEN_TYPE cat = getCategory(node->type);
	switch(cat){
		case KWD: return generate_KWD(node, lst, table);
		case OP: return generate_OP(node, lst, table);
		case IDENT: return generate_IDENT(node, lst, table);
		case LITERAL: return generate_LITERAL(node, lst, table);
		case CONV: return generate_CONV(node, lst, table);
		case BLOCK:
			   for(int i = 0; i<node->children_count; i++)
				   generate(node->children[i], lst, node->table);
			return (struct TACArg){0};
		default: return (struct TACArg){0};
	}

}

void gen_tac(struct Stream* in, struct TACList* lst, struct SymbolTable* table){
	struct StreamIterator it = stream_getIterator(in);
	for(size_t i = 0; i<it.n; i++){
		generate(it.arr[i], lst, table);
	}

}
