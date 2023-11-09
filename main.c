#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define panic(fmt, ...) do { \
	fprintf(stderr, fmt "\n", __VA_ARGS__); \
	exit(1); \
} while(0)

#define error(...) panic("Error: " __VA_ARGS__)

char *read_file_contents(const char *filename) {
	FILE *fp = fopen(filename, "rb");
	if(!fp) error("Could not open file %s: %s", filename, strerror(errno));
	long sz; char *buf;

	fseek(fp, 0, SEEK_END);
	sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	buf = malloc(sz + 1);

	(void) fread(buf, 1, sz, fp); buf[sz] = 0;
	fclose(fp);

	return buf;
}

int main(int argc, char **argv) {
	if(argc != 2) panic("Usage: %s file.bf", argv[0]);

	char *code = read_file_contents(argv[1]);

	int ip = 0; char c; while(c = code[ip]) {
		switch(c) {
			case '.':
			case ',':
			case '+':
			case '-':
			case '<':
			case '>':
			case '[':
			case ']':
				putc(c, stdout);
		}
		++ip;
	}

	free(code);
}
