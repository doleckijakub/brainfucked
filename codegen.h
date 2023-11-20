#ifndef _CODEGEN_H
#define _CODEGEN_H

#include <stdio.h>

typedef struct {
	const char *output_filename;
	FILE *sink;
	size_t memory_buffer_len;
} Codegen;

Codegen codegen_init(const char *output_filename, size_t memory_buffer_len);
void codegen_free(Codegen*);

#define codegen_put_elf(codegen, ...) __impl__codegen_put_elf(codegen, (sizeof((int[]){ 0, ##__VA_ARGS__ }) / sizeof(int) - 1), ##__VA_ARGS__)
void __impl__codegen_put_elf(Codegen*, size_t count, ...);

void codegen_chmod_x(Codegen*);

#endif
