#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#define HOSTS_FILE "/etc/hosts"
#define HOSTS_FILE_COPY "/etc/hosts_prosh_copy"
#define MAX_ITERATIONS 1000000

pthread_t *start_productivity_mode(int minutes);
int exit_productivity_mode();
void show_status();
void *pmode_thread(void *input);
void kill_blocked_processes();
int block_domains();
int unblock_domains();
void print_error_message(char *message, int code);

char error_message[100];
pthread_t pmode_thread_id;

struct args {
	bool productivity_mode_running;
	time_t productivity_mode_start_time;
	int productivity_mode_duration;
};

struct args *pmode_args;

pthread_t *start_productivity_mode(int minutes) {
	if (pmode_args == NULL) {
		pmode_args = (struct args *) malloc(sizeof(struct args));
	}

	if (pmode_args->productivity_mode_running) {
		strcpy(error_message, "Productivity mode already on.\n");
		return NULL;
	} else {
		strcpy(error_message, "");
		pmode_args->productivity_mode_running = true;
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
	int error = block_domains();
	if (error != 0) {
		print_error_message("Blocking domains failed.", error);
		thread_args->productivity_mode_running = false;
	}

	while (thread_args->productivity_mode_running) {
		if (XPending(display) > 0) {
			XNextEvent(display, &e);
			if (e.type == CreateNotify) {
				kill_blocked_processes();
			}
		}

		time(&now);
		if (difftime(now, thread_args->productivity_mode_start_time) > thread_args->productivity_mode_duration) {
			thread_args->productivity_mode_running = false;
			error = unblock_domains();
			if (error != 0) {
				print_error_message("Unblocking domains failed.", error);
			}
		}
	}
}

void print_error_message(char *message, int code) {
	if (code != 0) {
		switch (code) {
			case ENOENT: printf("%s No such file or directory.\n", message); break;
			case EACCES: printf("%s Permission denied.\n", message); break;
			default: printf("%s Error code: %d\n", message, code); break;
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

int block_domains() {
	FILE *original_hosts_file = fopen(HOSTS_FILE, "a+"); // a+ for reading and appending
	if (!original_hosts_file) {
		return errno;
	}
	
	FILE *hosts_file_copy = fopen(HOSTS_FILE_COPY, "w");
	if (!hosts_file_copy) {
		fclose(original_hosts_file);
		return errno;
	}
	
	int ch;
	int iteration = 0;
	
	while ((ch = fgetc(original_hosts_file)) > 0 && iteration < MAX_ITERATIONS) {
		fputc(ch, hosts_file_copy);
		iteration++;
	}
	
	fputs("127.0.0.1 youtube.com\n", original_hosts_file);
	fputs("127.0.0.1 twitter.com\n", original_hosts_file);
	fputs("127.0.0.1 facebook.com\n", original_hosts_file);
	
	fclose(original_hosts_file);
	fclose(hosts_file_copy);
	
	return 0;
}

int unblock_domains() {
	if (remove(HOSTS_FILE) != 0) {
		return errno;
	}
	
	if (rename(HOSTS_FILE_COPY, HOSTS_FILE) != 0) {
		return errno;
	}
	
	return 0;
}

int exit_productivity_mode() {
	// TODO check if user has permissions
	if (pmode_args != NULL) {
		pmode_args->productivity_mode_running = false;	
	}
}

void show_status() {
	if (pmode_args != NULL && pmode_args->productivity_mode_running) {
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
	
	pthread_t *thid = start_productivity_mode(30);
	if (thid == NULL) {
		printf("%s", error_message);
	} else {
		printf("Productivity mode activated.\n");
	}
	
	// exit_productivity_mode();
	
	pthread_join(*thid, NULL);
		
	printf("Productivity mode deactivated.\n");
	
	return 0;
}

