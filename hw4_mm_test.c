#include "hw4_mm_test.h"

int main()
{
	// printf("chunk_header size: %ld\n", sizeof(struct chunk_header));
	printf("4:\t0x%08" PRIXPTR "\n", (uintptr_t)hw_malloc(4));
	printf("12:\t0x%08" PRIXPTR "\n", (uintptr_t)hw_malloc(12));
	printf("20:\t0x%08" PRIXPTR "\n", (uintptr_t)hw_malloc(20));
	// printf("%s\n", hw_free(NULL) == 1 ? "success" : "fail");
	// printf("start_brk: %p\n", get_start_sbrk());
	watch_heap();
	// char input[20];
	// while (!feof(stdin)) {
	//     if (fgets(input, 20, stdin) != NULL) {
	//         size_t need = atoll(get_argv(input));
	//         printf("0x%08" PRIxPTR "\n", (uintptr_t)hw_malloc(need));
	//         // printf("%lld\n", atoll(get_argv(input)));
	//     }
	// }
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
