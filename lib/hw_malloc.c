#include "hw_malloc.h"
#include <stdio.h>
#include <stdlib.h>

void *hw_malloc(size_t bytes)
{
	if ((void *)start_sbrk == NULL) {
		start_sbrk = (chunk_ptr_t)sbrk(64 * 1024);
	}
	printf("%p\n", (void *)start_sbrk);
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
