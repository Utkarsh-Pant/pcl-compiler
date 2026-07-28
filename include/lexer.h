/**
 * @file lexer.c
 * @brief Compiler Lexer. Reads files and converts it to token stream.
 * @ingroup core
 *
 * @todo Add createToken_PUNC for processing of digraphs possibly.
 */

#ifndef LEXER_H
#define LEXER_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "stream.h"
#include "tokens.h"

#define INITIAL_STRING_SIZE 64

// #define LEXER_DEBUG

#define LEX_SUC EXIT_SUCCESS
#define LEX_FIN 242 // because, why not?
#define LEX_ERR EXIT_FAILURE

#define COMMENT_CHR '#'
#define CHKN_WS(x) (x!='\n' && x!= '\t' && x!= ' ')
#define CHK_WS(x) (x=='\n' || x=='\t' || x==' ')

/**
 * Main control loop for the lexer.
 *
 * @param[in] filename Input file to be read.
 * @param[out] output Externally initialized stream for the lexer to fill.
 *
 * @return EXIT_SUCCESS Or EXIT_FAILURE, based on success of lexing.
 */
int lex(const char filename[static 1], struct Stream* output);
#endif
