#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include "header.h"
#include "blacklist_manager.c"
#include "productivity_mode.h"
#include "productivity_mode.c"

// compile command needs -lreadline and -lX11 flag (gcc main.c -lreadline -lX11)

char history[MAX_HISTORY_LEN][MAX_LEN];
char readline_buffer[MAX_LEN + 16];
char *input_buffer;

enum Commands
{
	CD,
	LS,
	EXEC,
	PROD
};

enum ProshCommands
{
	START,
	END,
	STATUS,
	ADD,
	REMOVE,
	LIST
};

// progressively guess letter until message is printed
void print_welcome_message()
{
	char welcome_message[] = "Welcome to prosh [Version 1.00]";
	char c;
	int compare;
	int index = 0;

	srand(time(NULL));

	while (1)
	{
		c = "qwertyuiopasdfghjklzxcvbnmWelcome to prosh [Version 0.01]QWERTYUIOPASDFGHJKL"[rand() % 76]; // pick random symbol

		printf("\033[0;32m");
		printf("%c", c);
		printf("\033[0;37m");

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

int get_prosh_command_id()
{
	char *command = strtok(NULL, " ");

	if (strcmp(command, "start") == 0)
	{
		return START;
	}
	else if (strcmp(command, "end") == 0)
	{
		return END;
	}
	else if (strcmp(command, "status") == 0)
	{
		return STATUS;
	}
	else if (strcmp(command, "add") == 0)
	{
		return ADD;
	}
	else if (strcmp(command, "remove") == 0)
	{
		return REMOVE;
	}
	else if (strcmp(command, "list") == 0)
	{
		return LIST;
	}
	else
	{
		return -1;
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
	else if (strcmp(command, "prod") == 0)
	{
		return PROD;
	}
	else
	{
		return EXEC;
	}
}

// use chdir command
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

// print all values returned by scandir
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
			if (result_code > 1)
				printf("%d. %s\n", result_code - 1, list_of_files[result_code]->d_name);
			free(list_of_files[result_code]);
		}

		free(list_of_files);
		printf("\n");
	}
}

// create new thread to execute command and wait for it
void execute_file(char *argument, char **list_of_arguments)
{
	if (fork() == 0)
	{
		int result_code = execvp(argument, list_of_arguments);

		if (result_code < 0)
		{
			printf("Execution of %s failed\n", argument);
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

	// set up command history
	using_history();
	stifle_history(MAX_HISTORY_LEN);

	int command_id;
	char *argument;
	char *saveptr;

	char cwd[MAX_LEN]; // currentworking directory
	
	init_basic_blocked_domains();
	init_basic_blocked_processes();

	while (1)
	{
		getcwd(cwd, MAX_LEN);


		snprintf(readline_buffer, MAX_LEN + 16, "%s%s>%s", "\033[0;32m", cwd, "\033[0;37m");
		input_buffer = readline(readline_buffer);

		if (strlen(input_buffer) != 0) // if command not empty
		{
			add_history(input_buffer);

			char *command_copy = malloc(strlen(input_buffer) + 1); // need copy because strtok changes its argument
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
			case PROD:

				switch (get_prosh_command_id())
				{
				case START:
					char error_message[MAX_ERROR_LEN];

					pthread_t *tid = start_productivity_mode(error_message);

					if (tid == NULL)
					{
						printf("%s\n\n", error_message);
					}
					else
					{
						printf("Productivity mode started\n\n");
					}
					break;
				case END:
					char status_error_message[MAX_ERROR_LEN];
					exit_productivity_mode(status_error_message);
					if (status_error_message) {
						printf("%s\n\n", status_error_message);
					}
					break;
				case STATUS:
					show_status();
					printf("\n");
					break;
				case ADD:
					argument = strtok(NULL, " ");

					if (argument == NULL)
					{
						printf("prod add usage: prod add [process | domain] [name of process | domain]\n\n");
						break;
					}

					if (strcmp("domain", argument) == 0)
					{
						argument = strtok(NULL, " ");
						if (argument == NULL)
						{
							printf("prod add usage: prod add [process | domain] [name of process | domain]\n\n");
							break;
						}
						else if (pmode_args->productivity_mode_running)
						{
							printf("domains cannot be added while productivity mode is running\n\n");
							break;
						}

						add_blocked_domain(argument, true);
					}
					else if (strcmp("process", argument) == 0)
					{
						argument = strtok(NULL, " ");
						if (argument == NULL)
						{
							printf("prod add usage: prod add [process | domain] [name of process | domain]\n\n");
							break;
						}
						else if (pmode_args->productivity_mode_running)
						{
							printf("processes cannot be added while productivity mode is running\n\n");
							break;
						}

						add_blocked_process(argument, true);
					}
					else
					{
						printf("prod add usage: prod add [process | domain] [name of process | domain]\n\n");
					}
					break;
				case REMOVE:
					argument = strtok(NULL, " ");

					if (argument == NULL)
					{
						printf("prod remove usage: prod remove [process | domain] [name of process | domain]\n\n");
						break;
					}

					if (strcmp("domain", argument) == 0)
					{
						argument = strtok(NULL, " ");
						if (argument == NULL)
						{
							printf("prod remove usage: prod remove [process | domain] [name of process | domain]\n\n");
							break;
						}
						else if (pmode_args->productivity_mode_running)
						{
							printf("domains cannot be removed while productivity mode is running\n\n");
							break;
						}

						remove_blocked_domain(argument);
					}
					else if (strcmp("process", argument) == 0)
					{
						argument = strtok(NULL, " ");
						if (argument == NULL)
						{
							printf("prod remove usage: prod remove [process | domain] [name of process | domain]\n\n");
							break;
						}
						else if (pmode_args->productivity_mode_running)
						{
							printf("processes cannot be removed while productivity mode is running\n\n");
							break;
						}

						remove_blocked_process(argument);
					}
					else
					{
						printf("prod remove usage: prod remove [process | domain] [name of process | domain]\n\n");
					}
					break;
				case LIST:
					argument = strtok(NULL, " ");

					if (argument == NULL)
					{
						printf("prod list usage: prod list [process | domain]\n\n");
						break;
					}

					if (strcmp("domain", argument) == 0)
					{
						show_blocked_domains();
					}
					else if (strcmp("process", argument) == 0)
					{
						show_blocked_processes();
					}
					else
					{
						printf("prod list usage: prod list [process | domain]\n\n");
					}
					break;
				default:
					printf("prod command not found\n\n");
					break;
				}
				break;
			default:
				char *list_of_arguments[MAX_ARGUMENTS];
				int index = 0;

				argument = strtok(command_copy, " ");

				// add arguments to list until no more arguments
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

				list_of_arguments[index] = NULL; // last argument must be null

				execute_file(list_of_arguments[0], list_of_arguments);
				break;
			}

			free(command_copy); // free command copy
		}
	}
}
