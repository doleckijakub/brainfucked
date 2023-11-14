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

int main(int argc, char **argv) {
	if(argc != 2) panic("Usage: %s file.bf", argv[0]);

	const size_t memory_size = 256;

	char *code = read_file_contents(argv[1]);
	FILE *sink = stdout;

	#define put_nasm(fmt) fprintf(sink, fmt "\n")

	// nasm code
	put_nasm("global _start");
	put_nasm("section .text");
	put_nasm("_start:");

	// r11 will be used as pointer to current memory address
	put_nasm("mov r11, data");

	int ip = 0; char c; while(c = code[ip]) {
		switch(c) {
			case '+':
				put_nasm("inc dword [r11]"); break;
			case '-':
				put_nasm("dec dword [r11]"); break;
			case '<':
				put_nasm("dec r11"); break;
			case '>':
				put_nasm("inc r11"); break;
			case '.':
				put_nasm("mov rax, 1");
				put_nasm("mov rdi, 1");
				put_nasm("mov rsi, r11");
				put_nasm("mov rdx, 1");
				put_nasm("syscall");
				break;
			case ',':
			case '[':
			case ']':
				panic("%c unimplemented", c); break;
			default: ;
		}
		++ip;
	}

	put_nasm("mov rax, 60");
	put_nasm("mov rdi, 0");
	put_nasm("syscall");
	
	put_nasm("section .bss");
	put_nasm("data: resb 256");

	free(code);
}
