#include "hw4_mm_test.h"

int main()
{
	printf("chunk_header size: %ld\n", sizeof(struct chunk_header));
	// printf("8:\t%p\n", hw_malloc(8));
	printf("8:\t0x%08" PRIXPTR "\n", (uintptr_t)hw_malloc(8));
	printf("16:\t%p\n", hw_malloc(16));
	printf("24:\t%p\n", hw_malloc(24));
	printf("%s\n", hw_free(NULL) == 1 ? "success" : "fail");
	printf("start_brk: %p\n", get_start_sbrk());
	return 0;
}
