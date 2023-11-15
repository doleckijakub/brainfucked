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
	
	put_nasm("op_write:");
	put_nasm("mov rax, 1 ; .");
	put_nasm("mov rdi, 1");
	put_nasm("mov rsi, r15");
	put_nasm("mov rdx, 1");
	put_nasm("syscall");
	put_nasm("ret");
				
	put_nasm("op_read:");
	put_nasm("mov rax, 0 ; ,");
	put_nasm("mov rdi, 1");
	put_nasm("mov rsi, r15");
	put_nasm("mov rdx, 1");
	put_nasm("syscall");
	put_nasm("ret");

	put_nasm("_start:");

	// r15 will be used as pointer to current memory address
	put_nasm("mov r15, data");

	put_nasm("mov  eax, 16");
	put_nasm("mov  edi, 0");
	put_nasm("mov  esi, 0x5401");
	put_nasm("mov  rdx, termios");
	put_nasm("syscall");

	put_nasm("and byte [c_lflag], 0FDh"); // clear ICANON

	put_nasm("mov  eax, 16");
	put_nasm("mov  edi, 0");
	put_nasm("mov  esi, 0x5402");
	put_nasm("mov  rdx, termios");
	put_nasm("syscall");

	size_t ip = 0, scope_counter = 0; char c; while(c = code[ip]) {
		size_t last_ip, repeats;
		for(last_ip = ip; code[last_ip] == c; last_ip++);
		repeats = last_ip - ip;

		switch(c) {
			case '+':
				if(repeats == 1)
					put_nasm("inc byte [r15] ; +");
				else
					put_nasm("add byte [r15], %zu ; %zu +", repeats, repeats);
				ip = last_ip - 1;
				break;
			case '-':
				if(repeats == 1)
					put_nasm("dec byte [r15] ; -");
				else
					put_nasm("sub byte [r15], %zu ; %zu -", repeats, repeats);
				ip = last_ip - 1;
				break;
			case '<':
				if(repeats == 1)
					put_nasm("dec r15 ; <");
				else
					put_nasm("sub r15, %zu ; %zu <", repeats, repeats);
				ip = last_ip - 1;
				break;
			case '>':
				if(repeats == 1)
					put_nasm("inc r15 ; >");
				else
					put_nasm("add r15, %zu ; %zu >", repeats, repeats);
				ip = last_ip - 1;
				break;
			case '.':
				put_nasm("call op_write");
				break;
			case ',':
				put_nasm("call op_read");
				break;
			case '[':
				scope_stack_push(++scope_counter);
				put_nasm("o_%zu: ; [", scope_counter);
				put_nasm("cmp byte [r15], 0");
				put_nasm("jz c_%zu", scope_counter);
				break;
			case ']':
				size_t scope = scope_stack_pop();
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
	put_nasm("termios:");
	put_nasm("c_iflag resd 1");
	put_nasm("c_oflag resd 1");
	put_nasm("c_cflag resd 1");
	put_nasm("c_lflag resd 1");
	put_nasm("c_line  resb 1");
	put_nasm("c_cc    resb 19");

	#undef put_nasm

	free(code);
}
