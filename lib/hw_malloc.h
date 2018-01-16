#ifndef HW_MALLOC_H
#define HW_MALLOC_H

#include <stddef.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdbool.h>

#define DEBUG
#ifdef DEBUG
#define PRINTERR(s)\
    fprintf(stderr, "\033[0;32;31m""%d: %s""\033[m", __LINE__, s);
#else
#define PRINTERR(s);
#endif

typedef void *chunk_ptr_t;
typedef long long chunk_size_t;
typedef long long chunk_flag_t;
typedef struct chunk_header chunk_header;

struct chunk_header {
	chunk_ptr_t prev;
	chunk_ptr_t next;
	chunk_size_t chunk_size;
	chunk_size_t prev_chunk_size;
	chunk_flag_t prev_free_flag;
};

typedef struct bin_t {
	chunk_ptr_t prev;
	chunk_ptr_t next;
	int size;
} bin_t;

extern void *hw_malloc(size_t bytes);
extern int hw_free(void *mem);
extern void *get_start_brk(void);
extern void show_bin(const int i);
extern void watch_heap();

#endif
