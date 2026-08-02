#include "semantic_analyser.h"

typedef enum{
	INVALID,
	VALID,
	PROM_R,
	PROM_L
}OP_STAT;

static bool is_dtype[TOKEN_TYPE_COUNT] = {
	
	#define DTYPE_MAP(kwd, lit) [kwd] = true, 
	#include "semantic_mappings.def"
};

static TOKEN_TYPE get_literal_type(TOKEN_TYPE type){
	switch(type){
		#define DTYPE_MAP(kwd, lit) case lit: return kwd;
		#include "semantic_mappings.def"
		default: return ERR;
	}
}

static bool isRel(TOKEN_TYPE type){
	switch(type){
		#define RELATION_OP(op) case op: return true;
		#include "semantic_mappings.def"
		default: return false;
	}
}

static bool is_assignment_op[TOKEN_TYPE_COUNT] = {
	#define ASSIGNMENT_OP(op) [op] = true,
	#include "semantic_mappings.def"
};

static TOKEN_TYPE unary_map[TOKEN_TYPE_COUNT][TOKEN_TYPE_COUNT] = {
	#define UNARY_ALLOW(op, type, res) [op][type] = res,
	#include "semantic_mappings.def"
};

static OP_STAT binary_map[TOKEN_TYPE_COUNT][TOKEN_TYPE_COUNT][TOKEN_TYPE_COUNT] = {
	#define BINARY_ALLOW(op, type1, type2, stat) [op][type1][type2] = stat,
	#include "semantic_mappings.def"
};

static void inject_func(struct SymbolTable* table) {
	struct AST* nodePF = createAST(NULL);
	nodePF->type = IDENT;
	nodePF->value.s = strdup("printf");

	struct TableRecord* recPF = symtable_add(table, nodePF, KWD_INT);
    	recPF->is_func = true;
	recPF->data.f.arg_count = 1;
	recPF->data.f.is_extern = true;
	recPF->data.f.is_variadic = true;

	struct AST* scanPF = createAST(NULL);
	scanPF->type = IDENT;
	scanPF->value.s = strdup("scanf");

	struct TableRecord* scanPR = symtable_add(table, scanPF, KWD_INT);
    	scanPR->is_func = true;
	scanPR->data.f.arg_count = 1;
	scanPR->data.f.is_extern = true;
	scanPR->data.f.is_variadic = true;
}

static int assign(struct AST* node, struct SymbolTable* table, TOKEN_TYPE dtype){
	// Check if it already exists	
	if(symtable_get_local(table, node) != NULL ) return SYM_ERR;
	struct TableRecord* record = symtable_add(table, node, dtype);
	node->entry = record;
	if(table->parent == NULL) node->entry->data.v.is_global = true;
	if(node->children_count > 0) node->entry->data.v.assigned=true;
	

	return SYM_SUCC;

}

static int top_down_analyse(struct AST* node, struct SymbolTable* table){
	if(is_dtype[node->type]){ // Assignment.
	
		for(size_t i = 0; i< (node->children_count); i++){
			if( assign(node->children[i], table, node->type) == SYM_ERR ) return SYM_ERR;
		}

	}	

	if(node->type == FUNC){
		if(node->children_count < 1) return SYM_ERR;

		struct TableRecord* record = symtable_get(table, node->children[0]);	
		if(record == NULL || !record->is_func) return SYM_ERR;	
		
		size_t argCount = node->children_count -1;
		if(
			(record->data.f.is_variadic && record->data.f.arg_count>argCount) ||
			(!record->data.f.is_variadic && record->data.f.arg_count != argCount)
		) return SYM_ERR;

		/**
		 * WARNING! NO TYPE CHECKS FOR FUNCTION PARAMTERS YET!!!
		 */

		node->result_type = record->type;
		node->entry = record;
		return SYM_SUCC;

	}

	if(node->type == IDENT){
		struct TableRecord* record = symtable_get(table, node);
		// Check if variable is registered.
		if( record == NULL ) return SYM_ERR;

		// Check if it is initialized
		if(!record->is_func && !record->data.v.assigned) return SYM_ERR;
		
		// Bind
		node->entry = record;
		node->result_type = record->type;

	}

	return SYM_SUCC;
}

static int operator_check(struct AST* node, struct SymbolTable* table){
		
	if(is_unary(node->type)){ 
		if( 
			node->type == OP_PLUS_PLUS || node->type == OP_PLUS_PLUS_POST ||
			node->type == OP_MINUS_MINUS || node->type == OP_MINUS_MINUS_POST || node->type == OP_UN_BITWISE_AND
		){
			if(node->children[0]->type != IDENT)return SYM_ERR;
		}

	
		TOKEN_TYPE operand_type = node->children[0]->result_type;
		node->result_type = unary_map[node->type][operand_type];
		
		if(node->result_type == ERR) return SYM_ERR;
		if(node->result_type != operand_type){
			struct AST* convNode = createAST(NULL);
			convNode->type = CONV;
			convNode->result_type = node->result_type; // Also tells what to convert to.
			addChild(convNode, node->children[0]);
			node->children[0] = convNode;
		}

		return SYM_SUCC;
	}	
		

		
	OP_STAT res  = binary_map[node->type][node->children[0]->result_type][node->children[1]->result_type];
	if(res == INVALID) return SYM_ERR;
	if(res == VALID) {
		node->result_type = node->children[0]->result_type;
		if(isRel(node->type)) node->result_type = KWD_INT;
		return SYM_SUCC;
	}

	struct AST* valNode, *promNode;
	struct AST* convNode = createAST(NULL);
	if(res == PROM_L) {
		promNode = node->children[0];
		valNode = node->children[1];
		
		node->children[0] = convNode;

	} else{
		promNode = node->children[1];
		valNode = node->children[0];
		
		node->children[1] = convNode;
	}

	convNode->type = CONV;
	convNode->result_type = valNode->result_type;
	node->result_type = valNode->result_type;
	addChild(convNode, promNode);

	if(isRel(node->type)) node->result_type = KWD_INT;

	return SYM_SUCC;
}


static int analyse_AST(struct AST* node, struct SymbolTable* table){
	/**
	 * Going Top-Down
	 * Tasks to be performed:
	 * -> Populate symbol table using KWD
	 * -> Check for use of undeclared/unassigned identifiers.
	 */
	if( top_down_analyse(node, table) == SYM_ERR) return SYM_ERR;		


	if (node->type == BLOCK){
		node->table = symtable_init(table); // Create table with current table as parent.
		table = node->table; // Switch to local scope
	}

	if (is_dtype[node->type]) {
	    for (size_t i = 0; i < node->children_count; i++) {
		// Ignore declared idents
		struct AST* ident_child  = node->children[i];
		if (ident_child->children_count > 0) {
		    if (analyse_AST(ident_child->children[0], table) == SYM_ERR) return SYM_ERR;
		}
	    }
	    return SYM_SUCC; 
	}

	if(is_assignment_op[node->type]){
		// Manual lvalue rvalue checks
		if(node->children_count != 2) return SYM_ERR;
		//rvalue check
		if(analyse_AST(node->children[1], table) == SYM_ERR) return SYM_ERR;

		//lvalue check
		//
		//Record will be empty if node isnt ident aswell.
		struct AST* lvalue = node->children[0];
		if(lvalue->type != IDENT) return SYM_ERR;
		struct TableRecord* record = symtable_get(table, lvalue); 
		if(record == NULL) return SYM_ERR;

		record->data.v.assigned = true;
		lvalue->entry = record;
		lvalue->result_type = record->type;

		node->result_type = record->type;
		
		return operator_check(node, table);

	}

	for(size_t i = 0; i< (node->children_count) ; i++) 
		if( analyse_AST(node->children[i], table) == SYM_ERR) return SYM_ERR;	
	
	/**
	 * Going Bottom-Up
	 * Tasks to be performed:
	 *
	 */
	
	if(node->type == BLOCK){
		// Set Max Bytes of current table:
		if(table->total_bytes > table->max_bytes) table->max_bytes = table->total_bytes;

		// Check for parent propogation:
		if(table->parent != NULL && table->parent->max_bytes < table->max_bytes){
			table->parent->max_bytes = table->max_bytes;
		}
		
		printf("Local scope exit.\n");
		symtable_print(table);
		printf("\n");

		return SYM_SUCC;
	}


	// todo: wont block cause err????
	TOKEN_TYPE cat = getCategory(node->type);
	switch(cat){
		case OP: return operator_check(node, table);
		case IDENT: return SYM_SUCC;
		case KWD: return SYM_SUCC;
		case LITERAL: 
			node->result_type = get_literal_type(node->type);
			return SYM_SUCC;
		case FUNC: return SYM_SUCC;
		default: return SYM_ERR;
	}

	
}



int analyse(struct Stream* in, struct SymbolTable* table){
	if(table->parent == NULL) inject_func(table);
	
	struct StreamIterator st = stream_getIterator(in);
	for(size_t i = 0; i<st.n; i++){
		if(analyse_AST(st.arr[i], table) == SYM_ERR) return SYM_ERR;	
	}
		
	return SYM_SUCC;
}
