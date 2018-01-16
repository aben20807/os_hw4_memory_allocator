#include "hw4_mm_test.h"

int main()
{
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
				void *mem = (void *)(uintptr_t)strtol(get_argv(input), NULL, 16);
				printf("%s\n", hw_free(mem) == 1 ? "success" : "fail");
			} else if (input[0] == 'p' &&
			           input[1] == 'r' &&
			           input[2] == 'i' &&
			           input[3] == 'n' &&
			           input[4] == 't'
			          ) {
				int i = input[10] - '0';
				show_bin(i);
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
