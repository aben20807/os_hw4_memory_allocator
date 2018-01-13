#include "hw4_mm_test.h"

int main()
{
	printf("chunk_header size: %ld\n", sizeof(struct chunk_header));
	printf("4:\t0x%08" PRIXPTR "\n", (uintptr_t)hw_malloc(4));
	printf("12:\t0x%08" PRIXPTR "\n", (uintptr_t)hw_malloc(12));
	printf("20:\t0x%08" PRIXPTR "\n", (uintptr_t)hw_malloc(20));
	printf("%s\n", hw_free(NULL) == 1 ? "success" : "fail");
	printf("start_brk: %p\n", get_start_sbrk());
	return 0;
}
