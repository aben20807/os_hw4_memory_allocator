#ifndef HW_MALLOC_H
#define HW_MALLOC_H

#include <stddef.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdbool.h>

typedef void *chunk_ptr_t;
typedef void *chunk_ptr_t;
typedef void *chunk_size_t;
typedef void *chunk_size_t;
typedef void *chunk_flag_t;

struct chunk_header {
	chunk_ptr_t prev;
	chunk_ptr_t next;
	chunk_size_t chunk_size;
	chunk_size_t pre_chunk_size;
	chunk_flag_t prev_free_flag;
};

/*Global Variable*/
extern chunk_ptr_t start_sbrk;
extern bool has_init;

extern void *hw_malloc(size_t bytes);
extern int hw_free(void *mem);
extern void *get_start_sbrk(void);

#endif
