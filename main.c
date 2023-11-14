#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#define error(fmt, ...) fprintf(stderr, "Error: " fmt "\n", __VA_ARGS__)
#define warn(fmt, ...) fprintf(stderr, "Warning: " fmt "\n", __VA_ARGS__)
#define info(fmt, ...) fprintf(stderr, "Info: " fmt "\n", __VA_ARGS__)
#define debug(fmt, ...) fprintf(stderr, "Debug: " fmt "\n", __VA_ARGS__)

#define DEBUG(x, fmt, ...) debug("%s:%d: %s = " fmt, __FILE__, __LINE__, #x, __VA_ARGS__)

#define panic(fmt, ...) do { \
	fprintf(stderr, "Fatal error: " fmt "\n", __VA_ARGS__); \
	exit(1); \
} while(0)

char *read_file_contents(const char *filename) {
	FILE *fp = fopen(filename, "rb");
	if(!fp) error("Could not open file %s: %s", filename, strerror(errno));
	long sz; char *buf;

	fseek(fp, 0, SEEK_END);
	sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	buf = (char*) malloc(sz + 1);

	if(fread(buf, 1, sz, fp) != sz) goto on_fail;
	
	buf[sz] = 0;
	fclose(fp);

	return buf;

on_fail:

	free(buf);
	fclose(fp);

	error("Failed to read file %s: %s", filename, strerror(errno));

	return NULL;
}

#define SCOPE_STACK_CAP 1024
size_t scope_stack[SCOPE_STACK_CAP] = { 0 };
size_t scope_stack_len = 0;

void scope_stack_push(int x) {
	assert(scope_stack_len < SCOPE_STACK_CAP && "Stack overflow");
	scope_stack[scope_stack_len++] = x;
}

size_t scope_stack_pop(void) {
	assert(scope_stack_len > 0 && "Stack underflow");
	return scope_stack[--scope_stack_len];
}

int main(int argc, char **argv) {
	if(argc != 2) panic("Usage: %s file.bf", argv[0]);

	const size_t memory_size = 256;

	char *code = read_file_contents(argv[1]);
	FILE *sink = stdout;

	#define put_nasm(fmt, ...) fprintf(sink, fmt "\n", ##__VA_ARGS__)

	// nasm code
	put_nasm("global _start");
	put_nasm("section .text");
	put_nasm("_start:");

	// r15 will be used as pointer to current memory address
	put_nasm("mov r15, data");

	int ip = 0; size_t scope_counter = 0; char c; while(c = code[ip]) {
		switch(c) {
			case '+':
				put_nasm("inc byte [r15] ; +"); break;
			case '-':
				put_nasm("dec byte [r15] ; -"); break;
			case '<':
				put_nasm("dec r15 ; <"); break;
			case '>':
				put_nasm("inc r15 ; >"); break;
			case '.':
				put_nasm("mov rax, 1 ; .");
				put_nasm("mov rdi, 1");
				put_nasm("mov rsi, r15");
				put_nasm("mov rdx, 1");
				put_nasm("syscall");
				break;
			case ',':
				panic("%c unimplemented", c); break;
			case '[':
				scope_stack_push(++scope_counter);
				DEBUG(scope_counter, "%zu", scope_counter);
				put_nasm("o_%zu: ; [", scope_counter);
				put_nasm("cmp byte [r15], 0");
				put_nasm("jz c_%zu", scope_counter);
				break;
			case ']':
				size_t scope = scope_stack_pop();
				DEBUG(scope, "%zu", scope);
				put_nasm("c_%zu: ; ]", scope);
				put_nasm("cmp byte [r15], 0");
				put_nasm("jnz o_%zu", scope);
				break;
			// default: ;
		}
		++ip;
	}

	put_nasm("mov rax, 60");
	put_nasm("mov rdi, 0");
	put_nasm("syscall");
	
	put_nasm("section .bss");
	put_nasm("data: resb 256");

	#undef put_nasm

	free(code);
}
