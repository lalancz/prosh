#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_LEN 100
#define MAX_HISTORY_LEN 10

// compile command needs -lreadline flag (gcc main.c -lreadline)

char history[MAX_HISTORY_LEN][MAX_LEN];
char *input_buffer;

void print_welcome_message() {
	char welcome_message[] = "Welcome to prosh [Version 0.01]";
	char c;
	int compare;
	int index = 0;

	srand(time(NULL));

	while (1) {
		c = "qwertyuiopasdfghjklzxcvbnmWelcome to prosh [Version 0.01]"[rand() % 57];
		printf("%c", c);

		if (c == welcome_message[index]) {
			index++;
		} else {
			usleep(2500);
			printf("\b");
			fflush(stdout);
		}

		if (index == 31) {
			printf("\n");
			return;
		}
	}
}

int main()
{
	print_welcome_message();

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