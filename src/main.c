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
		c = "qwertyuiopasdfghjklzxcvbnmWelcome to prosh [Version 0.01]QWERTYUIOPASDFGHJKL"[rand() % 76];
		printf("%c", c);

		if (c == welcome_message[index]) {
			index++;
		} else {
			usleep(1000);
			printf("\b");
			fflush(stdout);
		}

		if (index == 31) {
			printf("\n");
			return;
		}
	}
}

int get_command_id(char* command_string) {
	char *command = strtok(command_string, " ");

	if (strcmp(command, "cd") == 0) {
		return 1;
	} else if (strcmp(command, "ls") == 0) {
		return 2;
	}
}

void change_directory(char* argument) {
	if (argument == NULL) {
		return;
	} else {
		int result_code = chdir(argument);

		if (result_code != 0)
			printf("Could not change directory to %s\n", argument);
	}
}

void list_directory() {

}

void execute_file() {

}

int main()
{
	print_welcome_message();

	using_history();
	stifle_history(MAX_HISTORY_LEN);

	int command_id;
	char* argument;

	char cwd[MAX_LEN];

	while (1)
	{
		getcwd(cwd, MAX_LEN);
		printf("%s", cwd);

		input_buffer = readline(">");

		if (strlen(input_buffer) != 0)
		{
			add_history(input_buffer);

			command_id = get_command_id(input_buffer);

			switch (command_id) {
				case 1:
					argument = strtok(NULL, " ");
					change_directory(argument);
					break;
				case 2:

				default:


			}
		}
	}
}