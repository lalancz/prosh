#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>

pthread_t *start_productivity_mode(int minutes);
int exit_productivity_mode();
void show_status();
void *pmode_thread(void *input);

char error_message[100];
pthread_t pmode_thread_id;

struct args {
	int productivity_mode_running;
	time_t productivity_mode_start_time;
	int productivity_mode_duration;
};

struct args *pmode_args;

pthread_t *start_productivity_mode(int minutes) {
	if (pmode_args == NULL) {
		pmode_args = (struct args *) malloc(sizeof(struct args));
	}

	if (pmode_args->productivity_mode_running == 1) {
		strcpy(error_message, "Productivity mode already on.\n");
		return NULL;
	} else {
		strcpy(error_message, "");
		pmode_args->productivity_mode_running = 1;
		time(&pmode_args->productivity_mode_start_time);
		pmode_args->productivity_mode_duration = minutes;
		
		if (pthread_create(&pmode_thread_id, NULL, pmode_thread, (void *)pmode_args) != 0) {
			strcpy(error_message, "Productivity mode could not be activated.\n");
			return NULL;
		}
		
		return &pmode_thread_id;
	}
}

void *pmode_thread(void *input) {
	time_t now;
	while (((struct args *)input)->productivity_mode_running == 1) {
		printf("Productivity mode running...\n");
		// TODO kill processes
		sleep(1);
		time(&now);
		if (difftime(now, ((struct args *)input)->productivity_mode_start_time) > ((struct args *)input)->productivity_mode_duration) {
			((struct args *)input)->productivity_mode_running = 0;
		}
	}
}

int exit_productivity_mode() {
	// TODO check if user has permissions
	if (pmode_args != NULL) {
		pmode_args->productivity_mode_running = 0;	
	}
}

void show_status() {
	if (pmode_args != NULL && pmode_args->productivity_mode_running == 1) {
		char time_str[50];
		char remaining_time_str[20];
		time_t now;
		strftime(time_str, 50, "%Y-%m-%d %H:%M", localtime(&pmode_args->productivity_mode_start_time));
		time(&now);
	
		printf("Productivity mode running: yes\n");
		printf("Started at: %s\n", time_str);
		printf("Remaining time: %d seconds\n", (int)difftime(now, pmode_args->productivity_mode_start_time));
	} else {
		printf("Productivity mode running: no\n");
	}
}

