#include "hw_malloc.h"
#include <stdio.h>
#include <stdlib.h>

bool has_init = false;
chunk_ptr_t start_sbrk = NULL;

void *hw_malloc(size_t bytes)
{
	if (!has_init) {
		start_sbrk = (chunk_ptr_t)sbrk(64 * 1024);
		printf("sbrk: %p\n", start_sbrk);
		has_init = true;
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
