#include "parser.h"

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

static struct Stream* INPUT_STREAM = NULL;

/* Consuming. Should only be used for strict must-matches! */
int match(TOKEN_TYPE type){
	if(!isEmptyStream(INPUT_STREAM) && ((struct Token*)advanceStream(INPUT_STREAM))->type == type) return PARSE_SUCC;
	return PARSE_ERR;
}

/*
 * Expression parser, uses two stacks to directly perform infix->AST conv..
 */
static int parse_expr(struct AST** out){
	if(isEmptyStream(INPUT_STREAM)) return PARSE_ERR;
	
	struct Generic_Stack* node_stk = createStack();
	struct Generic_Stack* op_stk = createStack();

	while(!isEmptyStream(INPUT_STREAM)){
		struct Token* top = (struct Token*) peekStream(INPUT_STREAM);
		TOKEN_TYPE cat = getCategory(top);
		if(cat != OP && cat != LITERAL && cat != IDENT && top->type!=PUNC_OPEN_PAR) break;
		if(top->type == PUNC_OPEN_PAR){
			advanceStream(INPUT_STREAM); //Consume '('
			if((parse_expr(out))==PARSE_ERR || match(PUNC_CLOSE_PAR)==PARSE_ERR) return PARSE_ERR;
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
				struct AST* new = createAST(top->type);
				struct AST* b1 = (struct AST*) pop(node_stk);
				struct AST* b2 = (struct AST*) pop(node_stk);

				if(!b1 || !b2)return PARSE_ERR;
				addChild(new,b2);
				addChild(new,b1);
				
				push(node_stk, new);
			}

			push(op_stk, top);
			advanceStream(INPUT_STREAM);
		}
		else{
			struct AST* new = createAST(top->type);
			new->val = top->value;
			push(node_stk, new);
			advanceStream(INPUT_STREAM);
		}
	
	}

	while(!isEmptyStack(op_stk)){
		struct Token* top = pop(op_stk);
		struct AST* new = createAST(top->type);
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

/*
 * Parsing statements.
 * Main function called by the parse loop. [Since a program in pcl is a group of statements]
 * Performs grammar-based dispatching.
 */
static int parse_stmt(struct AST** out){
	printf("parse stmt called\n");
	if(isEmptyStream(INPUT_STREAM)) return PARSE_ERR;

	// Statement may reduce to either empty... or expression.
	struct Token* leadingToken = (struct Token*) peekStream(INPUT_STREAM);
	
	
	// Option 1: Statement -> EMPTY;
	if(leadingToken->type == PUNC_SC){
		advanceStream(INPUT_STREAM);
		*out = NULL;
		return PARSE_SUCC;
	}

	// Option 2: Classic Statement
	if(getCategory(leadingToken) == KWD){
		printf("\nNot parsing statements yet.\n");
		switch(leadingToken->type){
			case KWD_INT:
				// ideally int can apply to anything such that root of ast is an =
		}
	}

	// Option 3: Try reducing statement to expression ending with ';'
	return (printf("Parsing Expr\n") && parse_expr(out) && match(PUNC_SC));	
}

/*
 * Parsing a block.
 * This function should only (ideally) be called from parse_stmt(), if a block is included in grammar. 
 */
static int parse_block(struct AST** out){
	if(isEmptyStream(INPUT_STREAM)) return PARSE_ERR;
	
	// Blocks are either empty... or statement.
	struct Token* leadingToken = (struct Token*) peekStream(INPUT_STREAM);

	// Option 1: Classic Block
	if(leadingToken->type == PUNC_OPEN_BRACE){
		advanceStream(INPUT_STREAM);
		if(isEmptyStream(INPUT_STREAM)) return PARSE_ERR;	
		struct Token* tk = (struct Token*) peekStream(INPUT_STREAM);
		while(!isEmptyStream(INPUT_STREAM) && tk->type != PUNC_CLOSE_BRACE){
			if(parse_stmt(out) == PARSE_ERR) return PARSE_ERR;
			if(isEmptyStream(INPUT_STREAM)) return PARSE_ERR;
			tk = (struct Token*) peekStream(INPUT_STREAM);
		}
		
		if (advanceStream(INPUT_STREAM) != NULL) return PARSE_SUCC;
		return PARSE_ERR;
	}

	// Option 2: Statement
	return parse_stmt(out);
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

	INPUT_STREAM = input;
	struct AST* node = NULL;
	while(!isEmptyStream(INPUT_STREAM)){
		node = NULL;
		if(parse_stmt(&node) == PARSE_SUCC){
			
			if(node!=NULL) appendStream(output, node);		
		} else {
			printf("PARSING ERROR!\n");
			return PARSE_ERR;
		}
	}

}
