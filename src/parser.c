#include "parser.h"

/*
 * Some ideas:
 * A ParserContext or something to wrap relevant data in future
 * Add a goto label for cleanup, and add it to all fail-paths for memmory cleanup and prevent leaks.
 * Make match not consume on failure?
 */

static PREC precidence_table[TOKEN_TYPE_COUNT]={
	[OP_PLUS_PLUS] = PREC_UNARY,
	[OP_MINUS_MINUS] = PREC_UNARY,
	[OP_NOT] = PREC_UNARY,

	[OP_MULTIPLY] = PREC_MULT,
	[OP_DIVIDE] = PREC_MULT,

	[OP_PLUS] = PREC_ADD,
	[OP_MINUS] = PREC_ADD,

	[OP_LESSER] = PREC_REL,
	[OP_GREATER] = PREC_REL,
	[OP_LESSER_EQUALS] = PREC_REL,
	[OP_GREATER_EQUALS] = PREC_REL,

	[OP_NOT_EQUALS] = PREC_EQ,
	[OP_EQUALS_EQUALS] = PREC_EQ,

	[OP_PLUS_EQUALS] = PREC_ASS,
	[OP_MINUS_EQUALS] = PREC_ASS,
	[OP_DIVIDE_EQUALS] = PREC_ASS,
	[OP_MULTIPLY_EQUALS] = PREC_ASS,
	[OP_EQUALS] = PREC_ASS,

	[OP_COMMA] = PREC_COMMA

};

static unsigned assignment_table[TOKEN_TYPE_COUNT]={
	[KWD_INT] = 1,
	[KWD_CHR] = 1
};

static int parse_expr(struct AST** out, struct Stream* in);
static int parse_stmt(struct AST** out, struct Stream* in);
static int parse_block(struct AST** out, struct Stream* in);

/* Consuming. Should only be used for strict must-matches! */
int match(TOKEN_TYPE type, struct Stream* in){
	if(!isEmptyStream(in) && ((struct Token*)advanceStream(in))->type == type) return PARSE_SUCC;
	return PARSE_ERR;
}

/*
 * Expression parser, uses two stacks to directly perform infix->AST conv..
 */
typedef enum{
	FLAG_NONE = 0,
	FLAG_DISALLOW_COMMA = 1 << 0
} ParseFlags;


static int parse_expr_engine(struct AST** out, struct Stream* in, uint8_t flags){
	if(isEmptyStream(in)) return PARSE_ERR;
	
	struct Generic_Stack* node_stk = createStack();
	struct Generic_Stack* op_stk = createStack();

	while(!isEmptyStream(in)){
		struct Token* top = (struct Token*) peekStream(in);
		TOKEN_TYPE cat = getCategory(top);
		if(cat != OP && cat != LITERAL && cat != IDENT && top->type!=PUNC_OPEN_PAR) break;
		if( (flags & FLAG_DISALLOW_COMMA) && top->type == OP_COMMA) break;
		if(top->type == PUNC_OPEN_PAR){
			advanceStream(in); //Consume '('
			if((parse_expr(out, in))==PARSE_ERR || match(PUNC_CLOSE_PAR, in)==PARSE_ERR) return PARSE_ERR;
			push(node_stk, *out);
			continue;
		}
		if(cat==OP){
			PREC p = precidence_table[top->type];
			while( 
			!isEmptyStack(op_stk) &&
			(
			 (p==PREC_ASS && p<precidence_table[((struct Token*)peek(op_stk))->type]) || 
			 (p!= PREC_ASS && p<=precidence_table[((struct Token*)peek(op_stk))->type])
			)
			)
			{
				struct Token* top = pop(op_stk);
				struct AST* new = createAST(top);
				struct AST* b1 = (struct AST*) pop(node_stk);
				struct AST* b2 = (struct AST*) pop(node_stk);

				if(!b1 || !b2)return PARSE_ERR;
				addChild(new,b2);
				addChild(new,b1);
				
				push(node_stk, new);
			}

			push(op_stk, top);
			advanceStream(in);
		}
		else{
			struct AST* new = createAST(top);
			new->val = top->value;
			push(node_stk, new);
			advanceStream(in);
		}
	
	}

	while(!isEmptyStack(op_stk)){
		struct Token* top = pop(op_stk);
		struct AST* new = createAST(top);
		struct AST* b1 = (struct AST*) pop(node_stk);
		struct AST* b2 = (struct AST*) pop(node_stk);

		if(!b1 || !b2) return PARSE_ERR;
		addChild(new,b2);
		addChild(new,b1);
		push(node_stk, new);
	}
	struct AST* result = pop(node_stk);
	if(!result || !isEmptyStack(node_stk))return PARSE_ERR;

	*out = result;
	return PARSE_SUCC;

}

static int parse_expr(struct AST** out, struct Stream* in){
	return parse_expr_engine(out, in, FLAG_NONE);
}

static int parse_assign_expr(struct AST** out, struct Stream* in){
	return	parse_expr_engine(out, in, FLAG_DISALLOW_COMMA);
}








static int parse_assign(struct AST** out, struct Stream* in){

	struct Token* leadingToken = (struct Token*) peekStream(in);
	if(!assignment_table[leadingToken->type]) return PARSE_ERR;
	struct AST* parent = createAST(leadingToken);
	advanceStream(in);


	while(!isEmptyStream(in)){
		struct AST* id = NULL;
		struct AST* val = NULL;

		leadingToken = (struct Token*) peekStream(in);
		if(leadingToken->type != IDENT) return PARSE_ERR;
		id = createAST(leadingToken);
		advanceStream(in);
		addChild(parent,id);

		leadingToken = (struct Token*) peekStream(in);
		if(leadingToken->type == PUNC_SC) break;
		if(leadingToken->type == OP_COMMA){
			advanceStream(in);
			continue;
		}

		if( match(OP_EQUALS, in) == PARSE_ERR || parse_assign_expr(&val, in) == PARSE_ERR ) return PARSE_ERR;
		addChild(id, val);

		leadingToken = (struct Token*) peekStream(in);
		if(leadingToken->type == PUNC_SC) break;
		if(leadingToken->type == OP_COMMA){
			advanceStream(in);
			continue;
		}
		return PARSE_ERR;
	}


	if( match(PUNC_SC, in) == PARSE_ERR || parent->children_count <1) return PARSE_ERR;

	*out = parent;
	return PARSE_SUCC;
}

static int parse_IF(struct AST** out, struct Stream* in){
	// IF AST:
	// KWD_IF
	// --->CONDITION
	// --->SUCCESS BLOCK
	// --->OPTIONAL ELSE BLOCK

	struct AST* parent = createAST(NULL);
	parent->type = KWD_IF;
	struct AST* cond = NULL;
	struct AST* sucBlk = NULL;
	struct AST* elseBlk = NULL;
	
	if(
		match(KWD_IF, in) == PARSE_ERR ||
		match(PUNC_OPEN_PAR, in) == PARSE_ERR ||
		parse_expr(&cond, in) == PARSE_ERR ||
		match(PUNC_CLOSE_PAR, in) == PARSE_ERR || 
		parse_block(&sucBlk, in) == PARSE_ERR	
	
	) return PARSE_ERR;


	struct Token* leadingToken = (struct Token*) peekStream(in);
	if(leadingToken->type == KWD_ELSE){
		advanceStream(in);
		if(parse_block(&elseBlk,in) == PARSE_ERR) return PARSE_ERR;
	}
	addChild(parent, cond);
	addChild(parent, sucBlk);
	if(elseBlk != NULL) addChild(parent, elseBlk);
	

	*out = parent;
	return PARSE_SUCC;
}

static int parse_WHILE(struct AST** out, struct Stream* in){
	
	// WHILE AST:
	// KWD_WHILE
	// ---> CONDITION
	// ---> SUCCESS BLOCK

	struct AST* parent = createAST(NULL);
	parent->type = KWD_WHILE;
	struct AST* cond = NULL;
	struct AST* sucBlk = NULL;

	if(
		match(KWD_WHILE, in) == PARSE_ERR ||
		match(PUNC_OPEN_PAR, in) == PARSE_ERR ||
		parse_expr(&cond, in) == PARSE_ERR ||
		match(PUNC_CLOSE_PAR, in) == PARSE_ERR	||
		parse_block(&sucBlk, in) == PARSE_ERR
	
	) return PARSE_ERR;

	addChild(parent, cond);
	addChild(parent, sucBlk);

	*out = parent;
	return PARSE_SUCC;
}

static int parse_FOR(struct AST** out, struct Stream* in){
	// FOR STATEMENTS ARE REDUCED TO WHILE ONLY!!!!!
	// AST LOOKS LIKE:
	// BLOCK
	// ---> AssignmentStatement
	// ---> KWD_WHILE
	// ---  ---> CONDITION
	// ---  ---> BLOCK
	// ---  ---  ---> BLOCK (BODY)
	// ---  ---  ---> UPDATION

	struct AST* parent = NULL;
	struct AST* ass = NULL;
	struct AST* cond = NULL;
	struct AST* upd = NULL;
	struct AST* bodyBlk = NULL;
	if( 
		match(KWD_FOR, in) == PARSE_ERR ||	
		match(PUNC_OPEN_PAR, in) == PARSE_ERR
	) return PARSE_ERR;


	struct Token* leadingToken = (struct Token*) peekStream(in);
	if(leadingToken->type == PUNC_SC) advanceStream(in);
	else if( parse_assign(&ass, in) == PARSE_ERR ) return PARSE_ERR;

	leadingToken = (struct Token*) peekStream(in);
	if(leadingToken->type == PUNC_SC) advanceStream(in);
	else if( parse_expr(&cond, in) == PARSE_ERR  || match(PUNC_SC, in) == PARSE_ERR) return PARSE_ERR;

	leadingToken = (struct Token*) peekStream(in);
	if(leadingToken->type == PUNC_CLOSE_PAR) advanceStream(in);
	else if( parse_expr(&upd, in) == PARSE_ERR  || match(PUNC_CLOSE_PAR, in) == PARSE_ERR) return PARSE_ERR;

	if( parse_block(&bodyBlk, in) == PARSE_ERR ) return PARSE_ERR;

	struct AST* w = createAST(NULL);
	w->type = KWD_WHILE;
	if(cond == NULL){
		cond = createAST(NULL);
	       	cond->type = LITERAL_INT;
		cond->val.i = 1;	
	}

	addChild(w, cond);
	if(upd==NULL) addChild(w, bodyBlk);
	else{
		struct AST* b = createAST(NULL);
		b->type = BLOCK;
		addChild(b, bodyBlk);
		addChild(b, upd);
		addChild(w,b);
	}

	if(ass==NULL){
		*out = w;
		return PARSE_SUCC;
	}

	*out = createAST(NULL);
	(*out)->type = BLOCK;
	addChild(*out, ass);
	addChild(*out, w);
	return PARSE_SUCC;

}

/*
 * Parsing statements.
 * Main function called by the parse loop. [Since a program in pcl is a group of statements]
 * Performs grammar-based dispatching.
 */
static int parse_stmt(struct AST** out, struct Stream* in){
	if(isEmptyStream(in)) return PARSE_ERR;

	// Statement may reduce to either empty... or expression.
	struct Token* leadingToken = (struct Token*) peekStream(in);
	
	
	// Option 1: Statement -> EMPTY;
	if(leadingToken->type == PUNC_SC){
		advanceStream(in);
		*out = NULL;
		return PARSE_SUCC;
	}

	// Option 2: Classic Statement
	if(getCategory(leadingToken) == KWD){
		
		// Current Plan: Try others, default case sends it to being an assingment statement, which will then do the rest.
		switch(leadingToken->type){
			case KWD_IF: return parse_IF(out, in);
			case KWD_WHILE: return parse_WHILE(out, in);
			case KWD_FOR: return parse_FOR(out, in);
			default: return parse_assign(out, in); // Bad idea, but eeeh, works gud enough since it does internally check 
		}
	}

	// Option 3: Block
	if(leadingToken->type == PUNC_OPEN_BRACE) return parse_block(out, in);

	// Option 4: Try reducing statement to expression ending with ';'
	if(parse_expr(out, in) == PARSE_ERR || match(PUNC_SC, in) == PARSE_ERR) return PARSE_ERR;
	return PARSE_SUCC;
}

/*
 * Parsing a block.
 * This function should only (ideally) be called from parse_stmt(), if a block is included in grammar. (?) not necessary. 
 */
static int parse_block(struct AST** out, struct Stream* in){
	if(isEmptyStream(in)) return PARSE_ERR;
	
	// Blocks are either empty... or statement.
	struct Token* leadingToken = (struct Token*) peekStream(in);

	// Option 1: Classic Block
	if(leadingToken->type == PUNC_OPEN_BRACE){
		advanceStream(in);
		if(isEmptyStream(in)) return PARSE_ERR;	
		struct Token* tk = (struct Token*) peekStream(in);
		
		struct AST* parent = createAST(NULL);
		parent->type = BLOCK;
		
		while(!isEmptyStream(in) && tk->type != PUNC_CLOSE_BRACE){
			struct AST* child_stmt = NULL;
			if(parse_stmt(&child_stmt, in) == PARSE_ERR) return PARSE_ERR;
			if(isEmptyStream(in)) return PARSE_ERR;
			tk = (struct Token*) peekStream(in);
	
			addChild(parent,child_stmt);
		}
		
		if (advanceStream(in) != NULL){
			*out = parent;
			return PARSE_SUCC;
		}
		return PARSE_ERR;
	}

	// Option 2: Statement
	return parse_stmt(out, in);
}



int parse(struct Stream* input, struct Stream* output){

	/*
	 * Current Plan:
	 * If first is kwd -> statement, otherwise expression.
	 * For statement, send it to parse_stmt and let it process.
	 * 
	 * Related to blocks and statements:
	 * Blocks are a group of statement.
	 * They may reduce entirely to statement.
	 * The surrounding {...} are handled inside the parse_block.
	 *
	 * Otherwise read till ';' and then send it to parse_expr
	*/

	struct AST* node = NULL;
	while(!isEmptyStream(input)){
		node = NULL;
		if(parse_stmt(&node, input) == PARSE_SUCC){	
			if(node!=NULL) appendStream(output, node);		
		} 
		else 	return PARSE_ERR;
	}

}
