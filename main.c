#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "logger.h"
#include "lexer.h"
#include "codegen.h"

#define next_arg() __impl__next_arg(&argc, &argv)
char *__impl__next_arg(int *argc, char ***argv) {
	assert(*argc > 0);
	char *arg = **argv;
	--*argc;
	++*argv;
	return arg;
}

void usage(char *argv0) {
	panic("Usage: %s [-o output_filename] [-m memory_buffer_size] filename", argv0);
}

#define streq(a, b) (strcmp(a, b) == 0)

#define SCOPE_STACK_CAP 1024
static size_t scope_stack[SCOPE_STACK_CAP] = { 0 };
static size_t scope_stack_len = 0;

static void scope_stack_push(int x) {
	assert(scope_stack_len < SCOPE_STACK_CAP && "Stack overflow");
	scope_stack[scope_stack_len++] = x;
}

static size_t scope_stack_pop(void) {
	assert(scope_stack_len > 0 && "Stack underflow");
	return scope_stack[--scope_stack_len];
}

int main(int argc, char **argv) {
	char *program_name = next_arg();

	struct {
		char *input_filename;
		char *output_filename;
		size_t memory_size;
	} config;

	config.input_filename = NULL;
	config.output_filename = "a.nasm";
	config.memory_size = 256;

	while(argc > 0) {
		char *arg = next_arg();

		if(streq(arg, "-o")) {
			if(argc == 0) {
				panic("`%s` expected a filename", arg);
			} else {
				arg = next_arg();
				config.output_filename = arg;
			}
		} else if(streq(arg, "-m")) {
			if(argc == 0) {
				panic("`%s` expected an integer", arg);
			} else {
				arg = next_arg();
				config.memory_size = atoi(arg);
			}
		} else {
			if(config.input_filename != NULL) {
				panic("More than one input files provided: %s, %s... For now %s only suports single file compilation", config.input_filename, arg, program_name);
			} else {
				config.input_filename = arg;
			}
		}
	}

	if(config.input_filename == NULL) panic("No input file provided");

	Lexer lexer = lexer_init(config.input_filename);
	Codegen codegen = codegen_init(config.output_filename, config.memory_size);

	codegen_put_nasm(&codegen, "global _start");
	codegen_put_nasm(&codegen, "section .text");
	
	codegen_put_nasm(&codegen, "op_write:");
	codegen_put_nasm(&codegen, "mov rax, 1 ; .");
	codegen_put_nasm(&codegen, "mov rdi, 1");
	codegen_put_nasm(&codegen, "mov rsi, r15");
	codegen_put_nasm(&codegen, "mov rdx, 1");
	codegen_put_nasm(&codegen, "syscall");
	codegen_put_nasm(&codegen, "ret");
				
	codegen_put_nasm(&codegen, "op_read:");
	codegen_put_nasm(&codegen, "mov rax, 0 ; ,");
	codegen_put_nasm(&codegen, "mov rdi, 1");
	codegen_put_nasm(&codegen, "mov rsi, r15");
	codegen_put_nasm(&codegen, "mov rdx, 1");
	codegen_put_nasm(&codegen, "syscall");
	codegen_put_nasm(&codegen, "ret");

	codegen_put_nasm(&codegen, "_start:");

	// r15 will be used as pointer to current memory address
	codegen_put_nasm(&codegen, "mov r15, data");

	codegen_put_nasm(&codegen, "mov  eax, 16");
	codegen_put_nasm(&codegen, "mov  edi, 0");
	codegen_put_nasm(&codegen, "mov  esi, 0x5401");
	codegen_put_nasm(&codegen, "mov  rdx, termios");
	codegen_put_nasm(&codegen, "syscall");

	codegen_put_nasm(&codegen, "and byte [c_lflag], 0FDh"); // clear ICANON

	codegen_put_nasm(&codegen, "mov  eax, 16");
	codegen_put_nasm(&codegen, "mov  edi, 0");
	codegen_put_nasm(&codegen, "mov  esi, 0x5402");
	codegen_put_nasm(&codegen, "mov  rdx, termios");
	codegen_put_nasm(&codegen, "syscall");

	bool eof = false;
	size_t scope_counter = 0;
	while(!eof) {
		Token token = lexer_next_token(&lexer);

		const char *add_inst = token.as.repeated.count == 1 ? "inc" : "add";
		const char *sub_inst = token.as.repeated.count == 1 ? "dec" : "sub";

		switch(token.type) {
			case TOKEN_NOP: break;
			case TOKEN_ADD:
			case TOKEN_SUB:
			case TOKEN_MVL:
			case TOKEN_MVR:
				codegen_put_nasm(&codegen, "%s %s %c %zu",
					token.type % 15 == 0 /* < and - */ ? sub_inst : add_inst,
					token.type < '/' /* + and - */ ? "byte [r15]" : "r15",
					token.as.repeated.count == 1 ? ';' : ',',
					token.as.repeated.count
					);
				break;
			case TOKEN_OPEN:
				scope_stack_push(++scope_counter);
				codegen_put_nasm(&codegen, "o_%zu: ; [", scope_counter);
				codegen_put_nasm(&codegen, "cmp byte [r15], 0");
				codegen_put_nasm(&codegen, "jz c_%zu", scope_counter);
				break;
			case TOKEN_CLOSE: {
					size_t scope = scope_stack_pop();
					codegen_put_nasm(&codegen, "c_%zu: ; ]", scope);
					codegen_put_nasm(&codegen, "cmp byte [r15], 0");
					codegen_put_nasm(&codegen, "jnz o_%zu", scope);
				} break;
			case TOKEN_STD_WRITE:
				codegen_put_nasm(&codegen, "call op_write");
				break;
			case TOKEN_STD_READ:
				codegen_put_nasm(&codegen, "call op_read");
				break;
			case TOKEN_EOF: eof = true; break;
		}
	}


	codegen_put_nasm(&codegen, "mov rax, 60");
	codegen_put_nasm(&codegen, "mov rdi, 0");
	codegen_put_nasm(&codegen, "syscall");
	
	codegen_put_nasm(&codegen, "section .bss");
	codegen_put_nasm(&codegen, "data: resb 256");
	codegen_put_nasm(&codegen, "termios:");
	codegen_put_nasm(&codegen, "c_iflag resd 1");
	codegen_put_nasm(&codegen, "c_oflag resd 1");
	codegen_put_nasm(&codegen, "c_cflag resd 1");
	codegen_put_nasm(&codegen, "c_lflag resd 1");
	codegen_put_nasm(&codegen, "c_line  resb 1");
	codegen_put_nasm(&codegen, "c_cc    resb 19");

	codegen_free(&codegen);
	lexer_free(&lexer);
}
