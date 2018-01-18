#include "hw4_mm_pthread_test.h"

#include <stdio.h>
void *print_hw_malloc(size_t bytes);

int main()
{
	int num = 5;
	while (num--) {
		pthread_t mythread;
		if (pthread_create(&mythread, NULL, print_hw_malloc(16), NULL)) {
			printf("error creating thread.");
			abort();
		}
	}
	return 0;
}

void *print_hw_malloc(size_t bytes)
{
	int num = 5;
	while (num--) {
		void *ptr = hw_malloc(bytes);
		if (ptr != NULL) {
			printf("0x%08" PRIxPTR "\n", (uintptr_t)ptr);
		} else {
			printf("%p\n", ptr);
		}
	}
	return NULL;
}
