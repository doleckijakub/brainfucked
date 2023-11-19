#ifndef _LOGGER_H
#define _LOGGER_H

#include <stdio.h>
#include <stdlib.h>

#define error(fmt, ...) fprintf(stderr, "Error: " fmt "\n", ##__VA_ARGS__)
#define warn(fmt, ...) fprintf(stderr, "Warning: " fmt "\n", ##__VA_ARGS__)
#define info(fmt, ...) fprintf(stderr, "Info: " fmt "\n", ##__VA_ARGS__)
#define debug(fmt, ...) fprintf(stderr, "Debug: " fmt "\n", ##__VA_ARGS__)

#define DEBUG(x, fmt, ...) debug("%s:%d: %s = " fmt, __FILE__, __LINE__, #x, ##__VA_ARGS__)

#define panic(fmt, ...) do { \
	fprintf(stderr, "Fatal error: " fmt "\n", ##__VA_ARGS__); \
	exit(1); \
} while(0)

#endif
