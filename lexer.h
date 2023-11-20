#ifndef _LEXER_H
#define _LEXER_H

#include <stddef.h>

typedef struct {
	char *filename;
	size_t line, row;
} Location;

#define TOKEN_NOP 1
#define TOKEN_ADD '+'
#define TOKEN_SUB '-'
#define TOKEN_MVL '<'
#define TOKEN_MVR '>'
#define TOKEN_OPEN '['
#define TOKEN_CLOSE ']'
#define TOKEN_STD_WRITE '.'
#define TOKEN_STD_READ ','
#define TOKEN_EOF '\0'

typedef struct {
	Location location;
	char type;
	union {
		struct {
			size_t count;
		} repeated; 
	} as;
} Token;

typedef struct {
	char *filename;

	char *code;
	size_t code_len;

	size_t cur, bol, row; // current char, index of beginning of last line, row index
} Lexer;

Lexer lexer_init(char *filename);
void lexer_free(Lexer*);
Token lexer_next_token(Lexer*);

#endif
