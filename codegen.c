#include "codegen.h"

#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <limits.h>

#include "logger.h"

Codegen codegen_init(const char *output_filename, size_t memory_buffer_len) {
	Codegen codegen = {
		.output_filename = output_filename,
		.sink = fopen(output_filename, "wb"),
		.memory_buffer_len = memory_buffer_len
	};

	if(codegen.sink == NULL) panic("Could not output to file %s: %s", output_filename, strerror(errno));

	return codegen;
}

void codegen_free(Codegen *codegen) {
	fclose(codegen->sink);
}

// void codegen_put_nasm(Codegen *codegen, const char *fmt, ...) {
//	va_list val;
//	va_start(val, fmt);
//	vfprintf(codegen->sink, fmt, val);
//	fputc('\n', codegen->sink);
//	va_end(val);
//}

void __impl__codegen_put_elf(Codegen *codegen, size_t count, ...) {
	va_list va;
	va_start(va, count);
	while(count--) fputc(va_arg(va, int), codegen->sink);
	va_end(va);
}

void codegen_chmod_x(Codegen *codegen) {
	struct stat st;
	int fd = fileno(codegen->sink);
	
	if(fstat(fd, &st) != 0) {
		panic("Could not stat %s, %s", codegen->output_filename, strerror(errno));
	} else {
		if(fchmod(fd, st.st_mode | S_IXUSR) != 0) {
			panic("Could not chmod %s, %s", codegen->output_filename, strerror(errno));
		}
	}
}
