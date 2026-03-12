#include <inttypes.h>
#include "stream.h"
#include "lexer.h"
#include "fnv.h"
#include "parser.h"
/*
 * Inorder to use the parser debugger tool, output should be a bfs.
 * Format for bfs would be:
 *
 * (NodeName/Val, ChildCount) (ChildNodeName/Val, ChildCount)...
 * (AST2NodeName....)
 * q
 *
 */
int notPCL(char* filename){
	int n = strlen(filename);
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

	for(int i = 2; i<=argc; i++){
		struct Stream* tkStream = createStream();
		struct Stream* treeStream = createStream();

		printf("\n\n\033[103m\033[30m--- %s ---\033[49m\033[39m\n", argv[i-1]);
		if(notPCL(argv[i-1])){
			printf("Invalid input file: %s\n", argv[i-1]);
			continue;
		}

		lex(argv[i-1], tkStream);
		
		struct StreamIterator itr = getIterator(tkStream);
		for(int i = 0; i< itr.n; i++){
			printToken((struct Token*)itr.arr[i]);	
		}
		printf("\n");
		parse(tkStream, treeStream);

		printf("Parse output:\n");
		itr = getIterator(treeStream);
		for(int i = 0; i<itr.n; i++){
			printAST((struct AST*)itr.arr[i]);
		}
		printf("\n");

	}
	return EXIT_SUCCESS;

}
