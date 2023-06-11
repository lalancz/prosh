#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "command.c"

#define MAX_LEN 100
#define MAX_HISTORY_LEN 10
#define MAX_ARGUMENTS 10
#define MAX_ERROR_LEN 100

// compile command needs -lreadline flag (gcc main.c -lreadline)

char history[MAX_HISTORY_LEN][MAX_LEN];
char *input_buffer;

enum Commands
{
	CD,
	LS,
	EXEC,
	STARTPROD,
	ENDPROD,
	PRODSTAT
};

void print_welcome_message()
{
	char welcome_message[] = "Welcome to prosh [Version 0.01]";
	char c;
	int compare;
	int index = 0;

	srand(time(NULL));

	while (1)
	{
		c = "qwertyuiopasdfghjklzxcvbnmWelcome to prosh [Version 0.01]QWERTYUIOPASDFGHJKL"[rand() % 76];
		printf("%c", c);

		if (c == welcome_message[index])
		{
			index++;
		}
		else
		{
			usleep(1000);
			printf("\b");
			fflush(stdout);
		}

		if (index == 31)
		{
			printf("\n\n");
			return;
		}
	}
}

int get_command_id(char *command_string)
{
	char *command = strtok(command_string, " ");

	if (strcmp(command, "cd") == 0)
	{
		return CD;
	}
	else if (strcmp(command, "ls") == 0)
	{
		return LS;
	}
	else if (strcmp(command, "startprod") == 0)
	{
		return STARTPROD;
	}
	else if (strcmp(command, "endprod") == 0)
	{
		return ENDPROD;
	}
	else if (strcmp(command, "statprod") == 0)
	{
		return PRODSTAT;
	}
	else
	{
		return EXEC;
	}
}

void change_directory(char *argument)
{
	if (argument == NULL)
	{
		return;
	}
	else
	{
		int result_code = chdir(argument);

		if (result_code != 0)
			printf("Could not change directory to %s\n", argument);
	}

	printf("\n");
}

void list_directory(char *argument)
{
	struct dirent **list_of_files;

	int result_code = scandir(argument, &list_of_files, NULL, alphasort);

	if (result_code < 0)
	{
		printf("Could not list files at %s\n\n", argument);
	}
	else
	{
		printf("\nDirectory of %s\n\n", argument);

		while (result_code--)
		{
			printf("%d. %s\n", result_code, list_of_files[result_code]->d_name);
			free(list_of_files[result_code]);
		}

		free(list_of_files);
		printf("\n");
	}
}

void execute_file(char *argument, char **list_of_arguments)
{
	if (fork() == 0)
	{
		int result_code = execvp(argument, list_of_arguments);

		if (result_code < 0)
		{
			printf("Execution of %s failed\n\n", argument);
		}
	}
	else
	{
		wait(NULL);
	}

	printf("\n");
}

int main()
{
	print_welcome_message();

	using_history();
	stifle_history(MAX_HISTORY_LEN);

	int command_id;
	char *argument;
	char *saveptr;

	char cwd[MAX_LEN];

	while (1)
	{
		getcwd(cwd, MAX_LEN);

		input_buffer = readline(strcat(cwd, ">"));

		if (strlen(input_buffer) != 0)
		{
			add_history(input_buffer);

			char *command_copy = malloc(strlen(input_buffer) + 1);
			strcpy(command_copy, input_buffer);

			command_id = get_command_id(input_buffer);

			switch (command_id)
			{
			case CD:
				argument = strtok(NULL, " ");
				change_directory(argument);
				break;
			case LS:
				argument = strtok(NULL, " ");

				if (argument == NULL)
				{
					list_directory(".");
				}
				else
				{
					list_directory(argument);
				}
				break;
			case STARTPROD:
				argument = strtok(NULL, " ");
				int minutes = atoi(argument);

				char error_message[MAX_ERROR_LEN];

				pthread_t *tid = start_productivity_mode(minutes, error_message);

				if (tid == NULL)
				{
					printf("%s\n\n", error_message);
				}
				else
				{
					printf("Productivity started\n\n");
				}
				break;
			case ENDPROD:
				exit_productivity_mode();
				break;
			case PRODSTAT:
				show_status();
				break;
			default:
				char *list_of_arguments[MAX_ARGUMENTS];
				int index = 0;

				argument = strtok(command_copy, " ");

				while (argument != NULL)
				{
					if (index > MAX_ARGUMENTS)
					{
						printf("Maximum amount of arguments reached\n\n");
						break;
					}

					list_of_arguments[index++] = argument;

					argument = strtok(NULL, " ");
				}

				list_of_arguments[index] = NULL;

				execute_file(list_of_arguments[0], list_of_arguments);
				break;
			}

			free(command_copy);
		}
	}
}