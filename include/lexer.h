#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "stream.h"
#include "tokens.h"

#ifndef LEXER_H
#define LEXER_H

#define INITIAL_STRING_SIZE 64

// #define LEXER_DEBUG

#define UPD_SUC EXIT_SUCCESS
#define UPD_FIN 242 
//242 because not : )
#define UPD_ERR EXIT_FAILURE

#define COMMENT_CHR '#'
#define CHKN_WS(x) (x!='\n' && x!= '\t' && x!= ' ')
#define CHK_WS(x) (x=='\n' || x=='\t' || x==' ')

int lex(char[static 1], struct Stream*);
#endif
