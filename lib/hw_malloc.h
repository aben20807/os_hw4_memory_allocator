#ifndef HW_MALLOC_H
#define HW_MALLOC_H

#include <stddef.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdbool.h>

typedef void *chunk_ptr_t;
typedef long long chunk_size_t;
typedef long long chunk_flag_t;
typedef struct chunk_header chunk_header;

struct chunk_header {
	chunk_ptr_t prev;
	chunk_ptr_t next;
	chunk_size_t chunk_size;
	chunk_size_t pre_chunk_size;
	chunk_flag_t prev_free_flag;
};

struct bin_t {
	chunk_ptr_t prev;
	chunk_ptr_t next;
};

/*Global Variable*/
extern chunk_ptr_t start_sbrk;
extern bool has_init;
extern struct bin_t bin[7];

extern void *hw_malloc(size_t bytes);
extern int hw_free(void *mem);
extern void *get_start_sbrk(void);

static chunk_header *create_chunk(const chunk_size_t size);
static void split();
static void en_bin(struct chunk_header c);
static struct chunk_header de_bin();

#endif
