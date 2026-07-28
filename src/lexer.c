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
 *
 * ******
 * Have decided to switch to this method. Will change my lexer acc.
 * Creating a global token map that maps directly to verbose types.
 * Creating a get_category(...) function that maps them to broad types.
 * Create a is_fast(...) function and area to declare fast-break tokens.
 * ******
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


// Checks if a identifier can be promoted to a keyword.
// Otherwise does nothing.
static void chkPromote(struct Token* tk){
	struct{
		const char* name;
		TOKEN_TYPE kwd;
	} dispatchTable[] =
	{
		#define INIT(a,b)
		#define TERM(a)
		#define FAST(a,b)
		#define KMAP(str, kwd) {str, kwd},
		#include "token_mappings.def"
		#undef INIT
		#undef TERM
		#undef FAST
		#undef KMAP
		{NULL, 0} // Ensure last entry is always NULL.
	
	};	

	for(int i = 0; dispatchTable[i].name != NULL; i++){
		if(!strcmp(tk->value.s, dispatchTable[i].name)){
			
			free(tk->value.s);
			tk->value.s = NULL; // Freeing prev stored value and changing to keyword.

			tk->type = dispatchTable[i].kwd;
			tk->cap = 0;
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

static struct Token* createToken_OP(TOKEN_TYPE type){
	struct Token* tk = malloc(sizeof(struct Token));
	tk->type = type;
	return tk;
}


// Redirects to appropriate function calls. Also skips whitespaces.
// For more Ref.: info-create-tokens
int createToken(TOKEN_TYPE type, char val, struct Token** tk){

	TOKEN_TYPE tokenCat = getCategory(type);

	if(tokenCat == WS) return LEX_SUC;
	else if(tokenCat  == IDENT) *tk=createToken_IDENT(val);
	else if(tokenCat == LITERAL) *tk = createToken_LITERAL(val);				 
	else if(tokenCat == OP) *tk = createToken_OP(type);
	else if(tokenCat == PUNC){
		perror("Crticial Error: Unable to process multi-character punctuations (fast-path). Create function.\n");
		return LEX_ERR;
	}
	else{
		perror("Critical Error: Invalid token type.");
		return LEX_ERR;

	}

	return LEX_SUC;
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
			if((tk->value.s) == NULL) return LEX_ERR;
			tk->cap = 2*tk->cap;
		}
		
		tk->value.s[v_len] = val;
		tk->value.s[v_len+1] = 0;

		return LEX_SUC;
		
	}

	if(CHKN_WS(val)) ungetc(val, fptr);
	return LEX_FIN;
}

static int updateToken_OP_PLUS(struct Token* tk, FILE* fptr, char val){
	if(val == '+'){
		tk->type = OP_PLUS_PLUS;
		return LEX_SUC;
	}
	else if (val == '='){
	       	tk->type = OP_PLUS_EQUALS;	
		return LEX_SUC;
	}
	if (CHKN_WS(val)) ungetc(val,fptr);

	return LEX_FIN;	

}


static int updateToken_OP_MINUS(struct Token* tk, FILE* fptr, char val){
	if(val == '-'){
		tk->type = OP_MINUS_MINUS;
		return LEX_SUC;
	}
	else if (val == '='){
	       	tk->type = OP_MINUS_EQUALS;	
		return LEX_SUC;
	}
	else if (val == '>'){
		tk->type = OP_ARROW;
		return LEX_SUC;
	}
	if (CHKN_WS(val)) ungetc(val,fptr);

	return LEX_FIN;	

}

static int updateToken_OP_DIVIDE(struct Token* tk, FILE* fptr, char val){
	if (val == '='){
	       	tk->type = OP_DIVIDE_EQUALS;	
		return LEX_SUC;
	}

	if (CHKN_WS(val)) ungetc(val,fptr);
	return LEX_FIN;	

}


static int updateToken_OP_MULTIPLY(struct Token* tk, FILE* fptr, char val){
	if (val == '='){
	       	tk->type = OP_MULTIPLY_EQUALS;	
		return LEX_SUC;
	}
	if (CHKN_WS(val)) ungetc(val,fptr);

	return LEX_FIN;	

}

static int updateToken_OP_EQUALS(struct Token* tk, FILE* fptr, char val){
	if (val == '='){
		tk->type = OP_EQUALS_EQUALS;
		return LEX_SUC;
	}

	if (CHKN_WS(val)) ungetc(val,fptr);
	return LEX_FIN;

}

static int updateToken_OP_LESSER(struct Token* tk, FILE* fptr, char val){
	if (val == '='){
		tk->type = OP_LESSER_EQUALS;
		return LEX_SUC;
	}
	else if(val == '<'){
		tk->type = OP_LSHIFT;
		return LEX_SUC;
	}

	if (CHKN_WS(val)) ungetc(val, fptr);
	return LEX_FIN;
}

static int updateToken_OP_GREATER(struct Token* tk, FILE* fptr, char val){
	if (val == '='){
		tk->type = OP_GREATER_EQUALS;
		return LEX_SUC;
	}
	else if(val == '>'){
		tk->type = OP_RSHIFT;
		return LEX_SUC;
	}

	if (CHKN_WS(val)) ungetc(val, fptr);
	return LEX_FIN;
}

static int updateToken_OP_NOT(struct Token* tk, FILE* fptr, char val){

	if (val == '='){
		tk->type = OP_NOT_EQUALS;
		return LEX_SUC;
	}

	if (CHKN_WS(val)) ungetc(val, fptr);
	return LEX_FIN;
}

static int updateToken_OP_BITWISE_AND(struct Token* tk, FILE* fptr, char val){

	if (val == '&'){
		tk->type = OP_AND;
		return LEX_FIN;
	}

	if (CHKN_WS(val)) ungetc(val, fptr);
	return LEX_FIN;
}

static int updateToken_OP_BITWISE_OR(struct Token* tk, FILE* fptr, char val){

	if (val == '|'){
		tk->type = OP_OR;
		return LEX_SUC;
	}

	if (CHKN_WS(val)) ungetc(val, fptr);
	return LEX_FIN;
}
static int updateToken_LITERAL_INT(struct Token* tk, FILE* fptr, char val){
	if (val >= '0' && val <= '9'){
		tk->value.i = tk->value.i*10 + (val-'0');
		return LEX_SUC;
	}

	if (CHKN_WS(val)) ungetc(val,fptr);
	return LEX_FIN;
}

static int updateToken_LITERAL_CHR(struct Token* tk, FILE* fptr, char val){
	if (tk->cap != 0){
		return (val=='\'') ? LEX_FIN : LEX_ERR;
	}
	if (val=='\'') return LEX_ERR;
	tk->cap = sizeof(char);
	tk->value.c = val;
	return LEX_SUC;
}

static int updateToken_LITERAL_STR(struct Token* tk, FILE* fptr, char val){
	if (val == '\"') return LEX_FIN;
	if(tk->cap == 0){ // First time initialization.
		tk->value.s = calloc(sizeof(char), INITIAL_STRING_SIZE);		
		tk->value.s[0] = val; // Null Terminator alrdy guranteed by calloc.
		tk->cap = INITIAL_STRING_SIZE;
		return LEX_SUC;
	}
	int n = strlen(tk->value.s);
	if(n == tk->cap-1){
		tk->value.s = reallocarray(tk->value.s, sizeof(char), tk->cap*2);
		tk->cap = tk->cap*2;
	}
	tk->value.s[n] = val;
	tk->value.s[n+1] = 0;
	return LEX_SUC;


}

static int updateToken_TERMINAL(struct Token* tk, FILE* fptr, char val){
	// This function should only be used for terminal tokens.
	// For example, += is terminal, there are no decisions to be made
	// (Other than consuming newline/space at best.
	if (CHKN_WS(val)) ungetc(val,fptr);
	return LEX_FIN;	
}

//Servers as a redirector. Uses a dispatch table for efficiency.
// For more, Ref.: info-update-tokens
static int updateToken(struct Token* tk, FILE* fptr, char val){

	//Function pointer array to serve as dispatch table.
	static int (*dispatchTable[TOKEN_TYPE_COUNT])(struct Token*, FILE*, char) = {
		#define INIT(sym, op) [op] = &updateToken_##op ,
		#define TERM(op) [op] = &updateToken_TERMINAL,
		#define FAST(a,b)
		#define KMAP(a,b)
		#include "token_mappings.def"
		#undef INIT
		#undef TERM
		#undef FAST
		#undef KMAP

		[LITERAL_INT] = &updateToken_LITERAL_INT,
		[LITERAL_CHR] = &updateToken_LITERAL_CHR,
		[LITERAL_STR] = &updateToken_LITERAL_STR
	};

	if(dispatchTable[tk->type] == NULL) return LEX_ERR;
	return dispatchTable[tk->type](tk,fptr,val);
}



/* ----------------
 * Main control loop for the lexer.
 * Contains initial token mapppings to create tokens.
 * Updates the tokens and handles the return signals.
 * ----------------
 */
int lex(const char filename[static 1], struct Stream* output){

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
	char tk_map[128] = {0};
	char fast_map[128] = {0}; // Create fast mapping.	
	
	// Initial Character set for identifieres.
	for(char c = 'a'; c<='z'; c++) tk_map[c] = IDENT;
	for(char c = 'A'; c<='Z'; c++) tk_map[c] = IDENT;
	// Initial characters for literals.
	for(char c = '0'; c<= '9'; c++) tk_map[c] = LITERAL;
	
	#define INIT(character, category) tk_map[character] = category;	
	#define TERM(a)
	#define FAST(character, category) \
	tk_map[character] = category; \
	fast_map[character] = 1;
	#define KMAP(str, kwd)  // Ignore Keyword mapping here.
	#include "token_mappings.def"
	#undef INIT
	#undef TERM
	#undef FAST
	#undef KMAP

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
			//Ignore Whitespaces.
			if(tk_map[c] == WS) continue;


			// Fast Path. Create token, be happy, end.
			if(fast_map[c]){
				current_token = (struct Token*) malloc(sizeof(struct Token));
				current_token->type = tk_map[c];
				stream_append(output, current_token);
				current_token = NULL;
				continue;
			}

			if(createToken(tk_map[c], c, &current_token) != LEX_SUC){
				perror("Lexer Error: Unable to create token intializiation (Perhaps missing token mapping?)\n");
				return EXIT_FAILURE;
			}
			
		}
		else{
			int status = updateToken(current_token, fptr, c);
			if(status == LEX_FIN){
				if(current_token->type == IDENT) chkPromote(current_token);
				stream_append(output, current_token);
				current_token = NULL;
			}
			else if(status == LEX_SUC) continue;
			else{ 
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

