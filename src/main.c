#include <unistd.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_LEN 100
#define MAX_HISTORY_LEN 10

char history[MAX_HISTORY_LEN][MAX_LEN];
char *input_buffer;

int main()
{
	printf("prosh [Version 0.01]\n");

	using_history();
	stifle_history(MAX_HISTORY_LEN);

	char cwd[MAX_LEN];
	getcwd(cwd, MAX_LEN);

	while (1)
	{
		printf("%s", cwd);

		input_buffer = readline(">");

		if (strlen(input_buffer) != 0)
		{
			add_history(input_buffer);
		}
	}
}