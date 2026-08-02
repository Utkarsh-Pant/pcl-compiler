/**
 * @file main.c
 * @brief Main Entry point to the compiler.
 * @ingroup core
 *
 *
 * @todo Use safer types.
 * @todo [main.c] Implement command-dispatcher to easily add/handle a flag-based system for later.
 * @todo Change file directory layout. For includes to have 2 subdirectories: include/helper for utilities and include/definitions for tokens, ast, etc. declarations and utilities
*/


#include <stdio.h>
#include <stdlib.h>

#include "stream.h"
#include "lexer.h"
#include "parser.h"
#include "semantic_analyser.h"
#include "tac.h"


/**
 * @brief Checks whether a given filename belongs to our extension (i.e. .pcl)
 *
 * @param filename String containing the filename to be checked.
 *
 * @return true if the file does not have the `.pcl` extension. false otherwise
 */
bool notPCL(const char* filename){
	if(filename == NULL) return true;

	size_t n = strlen(filename);
	return (n<5 || strcmp(filename + n-4, ".pcl"));
}


int main(int argc, char** argv){
	if(argc< 2){
		printf("PCL - Personal Computing Language\n");
		printf("Developed by: utkarsh_pant\n");
		printf("File Extension: .pcl\n");
		printf("Syntax: pcl <filename>\n");	
		return EXIT_SUCCESS;
	}

	for(int i = 1; i<argc; i++){
		struct Stream* tkStream = stream_init();
		struct Stream* treeStream = stream_init();
		
		if(notPCL(argv[i])){
			printf("Invalid input file: %s\n", argv[i]);
			continue;
		}

		printf("\n\n\033[103m\033[30m--- %s ---\033[49m\033[39m\n", argv[i]);
		
		lex(argv[i], tkStream);
		printf("Lexer O/P:\n");		
		struct StreamIterator itr = stream_getIterator(tkStream);
		for(int i = 0; i< itr.n; i++){
			printToken((struct Token*)itr.arr[i]);	
		}
		printf("\n\n");

		if(parse(tkStream, treeStream) != PARSE_SUCC){
			printf("Parse Err\n");
			continue;
		}
		printf("Parse output:\n");


		struct SymbolTable* table = symtable_init(NULL);
		analyse(treeStream, table);
		itr = stream_getIterator(treeStream);
		for(int i = 0; i<itr.n; i++){
			printAST((struct AST*)itr.arr[i]);
		}
		printf("\n");
	
		symtable_print(table);
	
		struct TACList* tac_yay = TACList_init();
		gen_tac(treeStream, tac_yay, table);

		for(int i = 0; i<tac_yay->n; i++){
			TAC_print(&(tac_yay->arr[i]));
		}
	}

	return EXIT_SUCCESS;

}
