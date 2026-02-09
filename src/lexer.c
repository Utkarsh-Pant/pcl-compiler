#include "lexer.h"
/* ----------------
 * Curiosity Box:
 * Contains ideas I am thinking about, but are not critical or even necessary for now.
 * But might be good to have later.
 * 
 * IMPORTANT: This code contains comments with [someID] for cross referencing. 
 * If you see a comment with just Ref.: some-id, search/jump to [some-id] for the root of that reference.
 * ----------------
 * [idea-tkmap]
 * Might increase extensibility for createToken, by making tkMap global, and inferring from there directly in case of operators. 
 * Instead of needing to add new operator in both tkMap, then createToken, then updateToken. We may simply map it in tkMap.
 * Can change it to signal based like update token to basically allow for single edit token creation/removal.
 *
 * However, doing this means surrounding all operators with some sentinels like OP_START/OP_END, to check if they fit the broad category. 
 * ---------------
 * [idea-int-radix]
 * One thing to be wondered about:
 * Might change design for literal ints. Currently the capacity simply represent number of digits stored.
 * Might instead use the capacity as a placeholder for the radix and support multi radix systems(?) 
 * so things like
 * 032_12 -> Represent 032(=>BASE 32) 12(=>VALUE = 1*32 + 2 = 34 IN DECIMAL)
 * ----------------
 * [idea-fast]
 * Changing create token to be return signal based like update. 
 * Then instead of current way of handling fast paths, we can simply
 * return a SIG_FAST or something, to immediately end the token there.
 *
 * Otherwise tokens like OP_COMMA, will be a inefficiently handled.
 * Although we can make them work too, but that will make the structure
 * polluted. Rather either refactor the entire process to signal based and clean later, than to just make it work for now. 
 *
 * So for now OP_COMMA is handled poorly, planning to fix that soon.
 */

void printToken(struct Token* tk){
	
	char* dispatchTable[TOKEN_TYPE_COUNT] = {NULL};

	dispatchTable[IDENT] = "IDENT";
	dispatchTable[OP_PLUS] = "OP_PLUS";
	dispatchTable[OP_MINUS] = "OP_MINUS";
	dispatchTable[OP_DIVIDE] = "OP_DIVIDE";
	dispatchTable[OP_MULTIPLY] = "OP_MULTIPLY";
	dispatchTable[OP_NOT] = "OP_NOT";

	dispatchTable[OP_PLUS_PLUS] = "OP_PLUS_PLUS";
	dispatchTable[OP_MINUS_MINUS] = "OP_MINUS_MINUS";
	
	dispatchTable[OP_PLUS_EQUALS] = "OP_PLUS_EQUALS";
	dispatchTable[OP_MINUS_EQUALS] = "OP_MINUS_EQUALS";
	dispatchTable[OP_DIVIDE_EQUALS] = "OP_DIVIDE_EQUALS";
	dispatchTable[OP_MULTIPLY_EQUALS] = "OP_MULTIPLY_EQUALS";
	dispatchTable[OP_EQUALS] = "OP_EQUALS";
	dispatchTable[OP_NOT_EQUALS] = "OP_NOT_EQUALS";

	dispatchTable[OP_EQUALS_EQUALS] = "OP_EQUALS_EQUALS";
	dispatchTable[OP_LESSER_EQUALS] = "OP_LESSER_EQUALS";
	dispatchTable[OP_GREATER_EQUALS] ="OP_GREATER_EQUALS";
	dispatchTable[OP_LESSER] = "OP_LESSER";
	dispatchTable[OP_GREATER] = "OP_GREATER";	
	
	dispatchTable[LITERAL_INT] = "LITERAL_INT";
	dispatchTable[LITERAL_CHR] = "LITERAL_CHR";
	dispatchTable[LITERAL_STR] = "LITERAL_STR";

	dispatchTable[KWD] = "KWD";
	dispatchTable[PUNC] = "PUNC";
	printf("(%s)", dispatchTable[tk->type] == NULL ? "INVALID" : dispatchTable[tk->type]);
}

// Checks if a identifier can be promoted to a keyword.
// Otherwise does nothing.
static void chkPromote(struct Token* tk){
	struct{
		const char* name;
		KEYWORD kwd;
	} dispatchTable[] =
	{
		{"if", KWD_IF},
		{"else", KWD_ELSE},
		{"int", KWD_INT},
		{"char", KWD_CHR},
		{"for", KWD_FOR},
		{"while", KWD_WHILE},
		{"break", KWD_BREAK},
		{"continue", KWD_CONTINUE},
		{NULL, 0} // Ensure last entry is always NULL.
	
	};	

	for(int i = 0; dispatchTable[i].name != NULL; i++){
		if(!strcmp(tk->value.s, dispatchTable[i].name)){
			
			free(tk->value.s);
			tk->value.s = NULL; // Freeing prev stored value and changing to keyword.

			tk->type = KWD;
			tk->cap = 0;
			tk->value.i = dispatchTable[i].kwd;
			break;
		}
	}
}
/* ----------------
 * [info-create-tokens]
 * TOKEN CREATION FUNCTIONS.
 * 
 * createToken_*(char val)
 * Create the token and assign necessary information.
 * Inputs are general categories, but they are mapped to verbose types. Ex. OP->OP_PLUS 
 *
 * createToken(TOKEN_TYPE type, char val, struct Token** tk)
 * Redirects to appropriate function calls. Also skips whitespaces.
 *
 * Small Change for future: Might change design to pass back return signal like update token functions.
 * This will be useful only if there are many 'single-character' tokens though.
 * ----------------
 */
static struct Token* createToken_IDENT(char val){

	struct Token* tk = malloc(sizeof(struct Token));
	tk->type = IDENT;
	tk->value.s  = calloc(sizeof(char), INITIAL_STRING_SIZE);			
	tk->value.s[0] = val; // Insert character. Null termination of string guranteed by calloc.
	tk->cap = INITIAL_STRING_SIZE;
	
	return tk;
}

static struct Token* createToken_OP(char val){
	/*
	 * For operators and punctuations, we do not bother with setting token value and capacity.
	 * Since they are directly inferred from the token type.
	 * Satisfied with simplicity for now. If number of operators increase, worth discussing change to dispatch-based.
	 */
	
	struct Token* tk = malloc(sizeof(struct Token));
	switch(val){
		case '+':
			tk->type = OP_PLUS;
			return tk;
		case '-':
			tk->type = OP_MINUS;
			return tk;
		case '/':
			tk->type = OP_DIVIDE;
			return tk;
		case '*':
			tk->type = OP_MULTIPLY;
			return tk;
		case '=':
			tk->type = OP_EQUALS;
			return tk;
		case '<':
			tk->type = OP_LESSER;
			return tk;
		case '>':
			tk->type = OP_GREATER;
			return tk;
		case '!':
			tk->type = OP_NOT;
			return tk;
		default: 
			free(tk);
			return NULL;
	}
	return NULL;
}

static struct Token* createToken_LITERAL(char val){
	
	struct Token* tk = malloc(sizeof(struct Token));
	if(val=='\''){
		// Value will be fed afterwards.
		tk->type = LITERAL_CHR;
		tk->cap = 0; // To denote it is not filled yet.
	}
	else if(val == '\"'){
		// Value will be fed afterwards.
		tk->type = LITERAL_STR;
		tk->cap = 0;
	}
	else if(val>='0' && val<='9'){
		tk->type = LITERAL_INT;
		tk->cap = 1; // Here capacity will denote the number of digits present.(?)
			     // Ref.: idea-int-radix
		tk->value.i = val-'0';
	}
	return tk;

}

static struct Token* createToken_PUNC(char c){
	struct Token* tk = malloc(sizeof(struct Token));
	tk->type = PUNC;
	tk->value.c = c;
	
	return tk;
}

// Redirects to appropriate function calls. Also skips whitespaces.
// For more Ref.: info-create-tokens
int createToken(TOKEN_TYPE type, char val, struct Token** tk){

	if(type  == IDENT) *tk=createToken_IDENT(val);
	else if(type == OP) *tk = createToken_OP(val);
	else if(type == LITERAL) *tk = createToken_LITERAL(val);				 
	else if(type == PUNC) *tk = createToken_PUNC(val);
	else if(type == WS) return 1;
	else{
		perror("Critical Error: Invalid token type.");
		return 0;

	}

	return 1;
}

/* ----------------
 * [info-update-tokens]
 * UPDATING TOKENS.
 *
 * updateToken_*(struct Token* tk, FILE* fptr, char val);
 * Promotes verbose token types further if possible based on next input. For example: OP_PLUS -> OP_PLUS_EQUALS
 * File descriptor needed to rollback or consume newlines.
 * value serves as the value to be updated.
 * 
 * updateToken(struct Token* tk, FILE* fptr, char val);
 * Servers as a redirector. Uses a dispatch table for efficiency.
 * Return update signals (ref. lexer.h) to be handled by the main loop.
 *
 * Note: updateToken_TERMINAL(...) is a placeholder that may be used for any operators which are not to be promoted further.
 * For example - OP_PLUS_PLUS cannot be promoted further, so it is a 'terminal token'.
 * ----------------
 */
static int updateToken_IDENT(struct Token* tk, FILE* fptr, char val){
	if( 
	  (val>='a' && val<='z') || 
	  (val>='A' && val<='Z') || 
	  (val=='_')  || 
	  (val>='0' && val<='9')	
	){
		int v_len = strlen(tk->value.s);
		if( v_len == tk->cap -1){
			tk->value.s = reallocarray(tk->value.s, sizeof(char), 2*(tk->cap));
			if((tk->value.s) == NULL) return UPD_ERR;
			tk->cap = 2*tk->cap;
		}
		
		tk->value.s[v_len] = val;
		tk->value.s[v_len+1] = 0;

		return UPD_SUC;
		
	}

	if(CHKN_WS(val)) ungetc(val, fptr);
	return UPD_FIN;
}

static int updateToken_OP_PLUS(struct Token* tk, FILE* fptr, char val){
	if(val == '+'){
		tk->type = OP_PLUS_PLUS;
		return UPD_SUC;
	}
	else if (val == '='){
	       	tk->type = OP_PLUS_EQUALS;	
		return UPD_SUC;
	}
	if (CHKN_WS(val)) ungetc(val,fptr);

	return UPD_FIN;	

}


static int updateToken_OP_MINUS(struct Token* tk, FILE* fptr, char val){
	if(val == '-'){
		tk->type = OP_MINUS_MINUS;
		return UPD_SUC;
	}
	else if (val == '='){
	       	tk->type = OP_MINUS_EQUALS;	
		return UPD_SUC;
	}
	if (CHKN_WS(val)) ungetc(val,fptr);

	return UPD_FIN;	

}

static int updateToken_OP_DIVIDE(struct Token* tk, FILE* fptr, char val){
	if (val == '='){
	       	tk->type = OP_DIVIDE_EQUALS;	
		return UPD_SUC;
	}

	if (CHKN_WS(val)) ungetc(val,fptr);
	return UPD_FIN;	

}


static int updateToken_OP_MULTIPLY(struct Token* tk, FILE* fptr, char val){
	if (val == '='){
	       	tk->type = OP_MULTIPLY_EQUALS;	
		return UPD_SUC;
	}
	if (CHKN_WS(val)) ungetc(val,fptr);

	return UPD_FIN;	

}

static int updateToken_OP_EQUALS(struct Token* tk, FILE* fptr, char val){
	if (val == '='){
		tk->type = OP_EQUALS_EQUALS;
		return UPD_SUC;
	}

	if (CHKN_WS(val)) ungetc(val,fptr);
	return UPD_FIN;

}

static int updateToken_OP_LESSER(struct Token* tk, FILE* fptr, char val){
	if (val == '='){
		tk->type = OP_LESSER_EQUALS;
		return UPD_FIN;
	}

	if (CHKN_WS(val)) ungetc(val, fptr);
	return UPD_FIN;
}

static int updateToken_OP_GREATER(struct Token* tk, FILE* fptr, char val){
	if (val == '='){
		tk->type = OP_GREATER_EQUALS;
		return UPD_FIN;
	}

	if (CHKN_WS(val)) ungetc(val, fptr);
	return UPD_FIN;
}

static int updateToken_OP_NOT(struct Token* tk, FILE* fptr, char val){

	if (val == '='){
		tk->type = OP_NOT_EQUALS;
		return UPD_FIN;
	}

	if (CHKN_WS(val)) ungetc(val, fptr);
	return UPD_FIN;
}

static int updateToken_LITERAL_INT(struct Token* tk, FILE* fptr, char val){
	if (val >= '0' && val <= '9'){
		tk->value.i = tk->value.i*10 + (val-'0');
		return UPD_SUC;
	}

	if (CHKN_WS(val)) ungetc(val,fptr);
	return UPD_FIN;
}

static int updateToken_LITERAL_CHR(struct Token* tk, FILE* fptr, char val){
	if (tk->cap != 0){
		return (val=='\'') ? UPD_FIN : UPD_ERR;
	}
	if (val=='\'') return UPD_ERR;
	tk->cap = sizeof(char);
	tk->value.c = val;
	return UPD_SUC;
}

static int updateToken_LITERAL_STR(struct Token* tk, FILE* fptr, char val){
	if (val == '\"') return UPD_FIN;
	if(tk->cap == 0){ // First time initialization.
		tk->value.s = calloc(sizeof(char), INITIAL_STRING_SIZE);		
		tk->value.s[0] = val; // Null Terminator alrdy guranteed by calloc.
		tk->cap = INITIAL_STRING_SIZE;
		return UPD_SUC;
	}
	int n = strlen(tk->value.s);
	if(n == tk->cap-1){
		tk->value.s = reallocarray(tk->value.s, sizeof(char), tk->cap*2);
		tk->cap = tk->cap*2;
	}
	tk->value.s[n] = val;
	tk->value.s[n+1] = 0;
	return UPD_SUC;


}

static int updateToken_TERMINAL(struct Token* tk, FILE* fptr, char val){
	// This function should only be used for terminal tokens.
	// For example, += is terminal, there are no decisions to be made
	// (Other than consuming newline/space at best.
	if (CHKN_WS(val)) ungetc(val,fptr);
	return UPD_FIN;	
}

//Servers as a redirector. Uses a dispatch table for efficiency.
// For more, Ref.: info-update-tokens
static int updateToken(struct Token* tk, FILE* fptr, char val){

	//Function pointer array to serve as dispatch table.
	static int (*dispatchTable[TOKEN_TYPE_COUNT])(struct Token*, FILE*, char) = {
		[IDENT] = &updateToken_IDENT,
		[OP_PLUS] = &updateToken_OP_PLUS,
		[OP_MINUS] = &updateToken_OP_MINUS,
		[OP_DIVIDE] = &updateToken_OP_DIVIDE,
		[OP_MULTIPLY] = &updateToken_OP_MULTIPLY,
		[OP_NOT] = &updateToken_OP_NOT,
		
		[OP_PLUS_PLUS] = &updateToken_TERMINAL,
		[OP_MINUS_MINUS] = &updateToken_TERMINAL,
		
		[OP_PLUS_EQUALS] = &updateToken_TERMINAL,
		[OP_MINUS_EQUALS] = &updateToken_TERMINAL,
		[OP_DIVIDE_EQUALS] = &updateToken_TERMINAL,
		[OP_MULTIPLY_EQUALS] = &updateToken_TERMINAL,
		[OP_EQUALS] = &updateToken_OP_EQUALS,
		[OP_NOT_EQUALS] = &updateToken_TERMINAL,

		[OP_EQUALS_EQUALS] = &updateToken_TERMINAL,
		[OP_LESSER_EQUALS] = &updateToken_TERMINAL,
		[OP_GREATER_EQUALS] = &updateToken_TERMINAL,
		[OP_LESSER] = &updateToken_OP_LESSER,
		[OP_GREATER] = &updateToken_OP_GREATER,
		
		[LITERAL_INT] = &updateToken_LITERAL_INT,
		[LITERAL_CHR] = &updateToken_LITERAL_CHR,
		[LITERAL_STR] = &updateToken_LITERAL_STR
	};

	if(dispatchTable[tk->type] == NULL) return UPD_ERR;
	return dispatchTable[tk->type](tk,fptr,val);
}



/* ----------------
 * Main control loop for the lexer.
 * Contains initial token mapppings to create tokens.
 * Updates the tokens and handles the return signals.
 * ----------------
 */
int lex(char filename[static 1], struct Stream* output){

	/*  -----------------
	 *  Creating a map.
	 *  Mapping the standard ASCII-character set.
	 *  This mapping should only be used for initial characters.
	 *  This mapping only maps them to their basic types, it is not verbose. 
	 *  They will be sorted later through the createToken method. 
	 *
	 *  Extra - Ref.: idea-tkmap
	 *  ----------------
	 */
	int tk_map[128] = {0};	
	
	// Initial Character set for identifieres.
	tk_map['_'] = IDENT;
	for(char c = 'a'; c<='z'; c++) tk_map[c] = IDENT;
	for(char c = 'A'; c<='Z'; c++) tk_map[c] = IDENT;

	// Initial characteres (broadly) for operators.
	tk_map['+'] = OP;
	tk_map['-'] = OP;
	tk_map['*'] = OP;
	tk_map['/'] = OP;
	tk_map['='] = OP;
	tk_map['<'] = OP;
	tk_map['>'] = OP;
	tk_map['!'] = OP;

	// Initial characters for literals.
	for(char c = '0'; c<= '9'; c++) tk_map[c] = LITERAL;
	tk_map['\''] = LITERAL;
	tk_map['\"'] = LITERAL;

	// White space characters.
	tk_map[' '] = WS;
	tk_map['\n'] = WS;
	tk_map['\t'] = WS;

	
	// Punctuations
	tk_map['('] = PUNC;
	tk_map[')'] = PUNC;
	tk_map[';'] = PUNC;
	tk_map['['] = PUNC;
	tk_map[']'] = PUNC;
	tk_map['{'] = PUNC;
	tk_map['}'] = PUNC;
	tk_map[','] = PUNC;
	// -------------------------------------------------
	
	FILE* fptr = fopen(filename, "r");
	if (fptr == NULL){
		perror("Critical Error: Unable to open file.");
		return EXIT_FAILURE;
	}


	char c;
	struct Token* current_token = NULL;

	while( (c = fgetc(fptr)) != EOF){
		if(c>128 || c<0){
			perror("Crticial Error: Unrecognized character. PCL only supports standard ASCII");
			return EXIT_FAILURE;
		} 

		// Easiest case to deal (for now)
		if (current_token == NULL){
			
			if(c==COMMENT_CHR){
				while( 
					((c=fgetc(fptr)) != '\n') && 
					c!= EOF 
				) continue;
				if(c==EOF)break;	
			
			}
			
			if(!createToken(tk_map[c], c, &current_token)){
				perror("Lexer Error.");
				return EXIT_FAILURE;
			}
			if (current_token && current_token->type == PUNC){
				appendStream(output, current_token);
				current_token = NULL;
			}	
			
		}
		else{
			int status = updateToken(current_token, fptr, c);
			if(status == UPD_FIN){
				if(current_token->type == IDENT) chkPromote(current_token);
				appendStream(output, current_token);
				current_token = NULL;
			}
			else if(status == UPD_SUC) continue;
			else{ // UPD_ERR or some other unexpected code.
				printf("Lexer Error!\n");
				return EXIT_FAILURE;
			}
		
		}	
	}

	if(current_token != NULL){
		printf("Lexer Error! Incomplete Token.\n");
		return EXIT_FAILURE;
	}


	return EXIT_SUCCESS;
}

