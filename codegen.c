#include "codegen.h"

#include <string.h>
#include <errno.h>
#include <stdarg.h>

#include "logger.h"

Codegen codegen_init(const char *output_filename, size_t memory_buffer_len) {
	Codegen codegen = {
		.sink = fopen(output_filename, "wb"),
		.memory_buffer_len = memory_buffer_len
	};

	if(codegen.sink == NULL) panic("Could not output to file %s: %s", output_filename, strerror(errno));

	return codegen;
}

void codegen_free(Codegen *codegen) {
	fclose(codegen->sink);
}

void codegen_put_nasm(Codegen *codegen, const char *fmt, ...) {
	va_list val;
	va_start(val, fmt);
	vfprintf(codegen->sink, fmt, val);
	fputc('\n', codegen->sink);
	va_end(val);
}
