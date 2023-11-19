#include "lexer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <ctype.h>

#include "logger.h"

// static void read_file_contents(const char *filename, char **buf, size_t *len) {
// }

Lexer lexer_init(char *filename) {
	Lexer lexer = {
		.filename = filename
	};

	FILE *fp = fopen(filename, "rb");
	if(!fp) panic("Could not open file %s: %s", filename, strerror(errno));

	fseek(fp, 0, SEEK_END); // TODO: check errno
	lexer.code_len = ftell(fp);
	fseek(fp, 0, SEEK_SET); // TODO: check errno
	
	lexer.code = (char*) malloc(lexer.code_len + 1);

	if(fread(lexer.code, 1, lexer.code_len, fp) != lexer.code_len) goto fail;

	lexer.code[lexer.code_len] = 0;
	fclose(fp);

	return lexer;

fail:

	free(lexer.code);
	fclose(fp);

	panic("Failed to read file %s: %s", filename, strerror(errno));
}

void lexer_free(Lexer *lexer) {
	free(lexer->code);
}

static bool lexer_not_empty(Lexer *lexer) { return lexer->cur < lexer->code_len; }
static inline bool lexer_empty(Lexer *lexer) { return !lexer_not_empty(lexer); }

static void lexer_chop_char(Lexer *lexer) {
	if(lexer_empty(lexer)) return;

	char c = lexer->code[lexer->cur++];
	if(c == '\n') {
		lexer->bol = lexer-> cur;
		lexer->row++;
	}
}

static void lexer_trim(Lexer *lexer) {
	while(lexer_not_empty(lexer) && isspace(lexer->code[lexer->cur])) lexer_chop_char(lexer);
}

Token lexer_next_token(Lexer *lexer) {
	lexer_trim(lexer);
	if(lexer_empty(lexer)) return (Token) { 0 }; // TOKEN_EOF
	
	Location location = (Location) {
		.filename = lexer->filename,
		.line = lexer->row,
		.row = lexer->cur - lexer->bol
	};

	char c = lexer->code[lexer->cur];
	lexer_chop_char(lexer);
	return (Token) {
		.location = location,
		.type = c
	};
}
