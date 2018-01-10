#include "hw_malloc.h"
#include <stdio.h>
#include <stdlib.h>

bool has_init = false;
chunk_ptr_t start_sbrk = NULL;
struct bin_t bin[7];

void *hw_malloc(size_t bytes)
{
	long long chunk_size = bytes + 40LL;
	if (!has_init) {
		has_init = true;
		for (int i = 0; i < 7; i++) {
			bin[i].prev = &bin[i];
			bin[i].next = &bin[i];
			printf("bin[%d].p: %p\n", i, bin[i].prev);
			printf("bin[%d].n: %p\n", i, bin[i].next);
		}
		start_sbrk = (chunk_ptr_t)sbrk(64 * 1024);
		printf("sbrk: %p\n", start_sbrk);
		if (64 * 1024 - chunk_size > 8) {
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

void put_in_bin(struct chunk_header c)
{
}
