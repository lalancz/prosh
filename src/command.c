#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

pthread_t *start_productivity_mode(int minutes);
int exit_productivity_mode();
void show_status();
void *pmode_thread(void *input);
void kill_blocked_processes();

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
	struct args *thread_args = (struct args *)input;
	
	time_t now;
	XEvent e;
	Display *display = XOpenDisplay(NULL);
	XSelectInput(display, DefaultRootWindow(display), SubstructureNotifyMask);
	
	kill_blocked_processes();

	while (thread_args->productivity_mode_running == 1) {
		if (XPending(display) > 0) {
			XNextEvent(display, &e);
			if (e.type == CreateNotify) {
				kill_blocked_processes();
			}
		}

		time(&now);
		if (difftime(now, thread_args->productivity_mode_start_time) > thread_args->productivity_mode_duration) {
			thread_args->productivity_mode_running = 0;
		}
	}
}

void kill_blocked_processes() {
	clock_t begin = clock();

	system("pkill -15 mines");
	system("pkill -15 mahjongg");

	clock_t end = clock();
	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("Blocked processes killed (in %f s)\n", time_spent);
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

// TODO remove main method after testing finished
int main() {
	printf("Started\n");
	
	show_status();
	
	pthread_t *thid = start_productivity_mode(8);
	if (thid == NULL) {
		printf("%s", error_message);
	} else {
		printf("Productivity mode activated.\n");
	}
	
	for (int i = 0; i < 4; i++) {
		sleep(1);
	}
	
	// exit_productivity_mode();
	
	pthread_join(*thid, NULL);
		
	printf("Productivity mode deactivated.\n");
	
	return 0;
}

