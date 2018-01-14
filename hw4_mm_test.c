#include "hw4_mm_test.h"

int main()
{
	/*
	   // printf("chunk_header size: %ld\n", sizeof(struct chunk_header));
	   printf("4:\t0x%08" PRIXPTR "\n", (uintptr_t)hw_malloc(4));
	   printf("12:\t0x%08" PRIXPTR "\n", (uintptr_t)hw_malloc(12));
	   printf("20:\t0x%08" PRIXPTR "\n", (uintptr_t)hw_malloc(20));
	   // watch_heap();
	   void *ptr = (void*)0x28;
	   printf("free:\t0x%08" PRIXPTR "\n", (uintptr_t)ptr);
	   printf("%s\n", hw_free(ptr) == 1 ? "success" : "fail");
	   ptr = (void*)0x25;
	   printf("free:\t0x%08" PRIXPTR "\n", (uintptr_t)ptr);
	   printf("%s\n", hw_free(ptr) == 1 ? "success" : "fail");
	   ptr = (void*)0x90;
	   printf("free:\t0x%08" PRIXPTR "\n", (uintptr_t)ptr);
	   printf("%s\n", hw_free(ptr) == 1 ? "success" : "fail");
	   ptr = (void*)0x00100000;
	   printf("free:\t0x%08" PRIXPTR "\n", (uintptr_t)ptr);
	   printf("%s\n", hw_free(ptr) == 1 ? "success" : "fail");
	   // watch_heap();
	printf("0x%08" PRIxPTR "\n", (uintptr_t)hw_malloc(16));
	printf("0x%08" PRIxPTR "\n", (uintptr_t)hw_malloc(16));
	printf("0x%08" PRIxPTR "\n", (uintptr_t)hw_malloc(16));
	printf("0x%08" PRIxPTR "\n", (uintptr_t)hw_malloc(16));
	void *ptr = (void*)0x28;
	printf("%s\n", hw_free(ptr) == 1 ? "success" : "fail");
	ptr = (void*)0x98;
	printf("%s\n", hw_free(ptr) == 1 ? "success" : "fail");
	show_bin(0);
	show_bin(1);
	show_bin(2);
	show_bin(3);
	show_bin(4);
	show_bin(5);
	show_bin(6);
	   */
	// printf("%s\n", hw_free(NULL) == 1 ? "success" : "fail");
	// printf("start_brk: %p\n", get_start_sbrk());
	char input[20];
	while (!feof(stdin)) {
		if (fgets(input, 20, stdin) != NULL) {
			if (input[0] == 'a' &&
			    input[1] == 'l' &&
			    input[2] == 'l' &&
			    input[3] == 'o' &&
			    input[4] == 'c'
			   ) {
				size_t need = atoll(get_argv(input));
				printf("0x%08" PRIxPTR "\n", (uintptr_t)hw_malloc(need));
			} else if (input[0] == 'f' &&
			           input[1] == 'r' &&
			           input[2] == 'e' &&
			           input[3] == 'e'
			          ) {
				watch_heap();
				void *mem = (void *)(uintptr_t)strtol(get_argv(input), NULL, 16);
				printf("free:\t0x%08" PRIXPTR "\n", (uintptr_t)mem);
				printf("%s\n", hw_free(mem) == 1 ? "success" : "fail");
			}
		}
	}
	return 0;
}

char *get_argv(const char *command)
{
	char delim[] = " ";
	char *s = (char *)strndup(command, 20);
	char *pos;
	if ((pos = strchr(s, '\n')) != NULL)
		* pos = '\0';
	// split
	char *token;
	int argc = 0;
	for (token = strsep(&s, delim); token != NULL; token = strsep(&s, delim)) {
		if (argc == 1) {
			return token;
		}
		argc++;
	}
	return "";
}
