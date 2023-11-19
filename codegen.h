#ifndef _CODEGEN_H
#define _CODEGEN_H

#include <stdio.h>

typedef struct {
	FILE *sink;
	size_t memory_buffer_len;
} Codegen;

Codegen codegen_init(const char *output_filename, size_t memory_buffer_len);
void codegen_free(Codegen*);

void codegen_put_nasm(Codegen*, const char*, ...);

#endif
