#include "hw_malloc.h"
#include <stdio.h>
#include <stdlib.h>

bool has_init = false;
chunk_ptr_t start_sbrk = NULL;
struct bin_t bin[7] = {};

void *hw_malloc(size_t bytes)
{
	long long chunk_size = bytes + 40LL;
	if (!has_init) {
		has_init = true;
		for (int i = 0; i < 7; i++) {
			bin[i].prev = &bin[i];
			bin[i].next = &bin[i];
		}
		start_sbrk = (chunk_ptr_t)sbrk(64 * 1024);
		chunk_header *s = create_chunk(64 * 1024);
		printf("sbrk: %p, size: %lli\n", start_sbrk, s->chunk_size);
		if (64 * 1024 - chunk_size > 8) {
			split();
		} else {
		}
	} else {
	}
	void *ptr = sbrk(0);
	printf("%p\n", ptr);
	return NULL;
}

int hw_free(void *mem)
{
	return 0;
}

void *get_start_sbrk(void)
{
	return (void *)start_sbrk;
}

static chunk_header *create_chunk(const chunk_size_t size)
{
	chunk_header *ret = calloc(1, sizeof(chunk_header));
	ret->chunk_size = size;
	ret->prev = NULL;
	ret->next = NULL;
	ret->pre_chunk_size = 0;
	ret->prev_free_flag = 0;
	return ret;
}

static void split()
{
}

static void en_bin(struct chunk_header c)
{
}

static chunk_header de_bin()
{
	// return NULL;
}
