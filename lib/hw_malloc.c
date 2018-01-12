#include "hw_malloc.h"
#include <stdio.h>
#include <stdlib.h>

bool has_init = false;
void *start_brk = NULL;
void *heap_brk = NULL;
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
		start_brk = sbrk(64 * 1024);
		heap_brk = start_brk;
		chunk_header *s = create_chunk(64 * 1024);
		printf("sbrk: %p, size: %lli\n", start_brk, s->chunk_size);
		if (64 * 1024 - chunk_size > 8) {
			chunk_header *c = split(s, chunk_size);
			printf("%lld\n", c->chunk_size);
			printf("%p, size: %lli\n", start_brk, s->chunk_size);
		} else {
		}
	} else {
	}
	// void *ptr = sbrk(0);
	// printf("%p\n", ptr);
	return NULL;
}

int hw_free(void *mem)
{
	return 0;
}

void *get_start_sbrk(void)
{
	return (void *)start_brk;
}

static chunk_header *create_chunk(const chunk_size_t size)
{
	chunk_header *ret = heap_brk;
	heap_brk += sizeof(chunk_header);
	ret->chunk_size = size;
	ret->prev = NULL;
	ret->next = NULL;
	ret->pre_chunk_size = 0;
	ret->prev_free_flag = 0;
	return ret;
}

static chunk_header *split(chunk_header *ori, const chunk_size_t need)
{
	ori->chunk_size -= need;
	ori->pre_chunk_size = need;
	ori->prev_free_flag = 0;
	chunk_header *ret = create_chunk(need);
	return ret;
}

static void en_bin(const int bin_num, chunk_header *c)
{
	bin[bin_num].next = c;
}

static chunk_header de_bin()
{
	// return NULL;
}
