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
#include "command.h"

#define HOSTS_FILE "/etc/hosts"
#define HOSTS_FILE_COPY "/etc/hosts_prosh_copy"
#define MAX_ITERATIONS 1000000

pthread_t pmode_thread_id;

struct args {
	bool productivity_mode_running;
	time_t productivity_mode_start_time;
	int productivity_mode_duration;
};

struct args *pmode_args;

pthread_t *start_productivity_mode(int minutes, char *error_message) {
	/*
	 * pmode_args is null on the first time the productivity mode is started.
	 * In that case the memory needs to be allocated.
	 */
	if (!pmode_args) {
		pmode_args = (struct args *) malloc(sizeof(struct args));
	}

	/* Do not start the productivity mode if it is already running. */
	if (pmode_args->productivity_mode_running) {
		strcpy(error_message, "Productivity mode already on.\n");
		return NULL;
	}
	
	/*
	 * If a browser has already opened a blocked domain,
	 * the blocking mechanism will not be effective.
	 * (Probably because of cashing.)
	 * Therefore we ask the user to close their browsers
	 * before running the productivity mode.
	 */
	if (is_browser_running()) {
		strcpy(error_message, "Please close all browsers before starting the producitivity mode.\n");
		return NULL;
	}
	
	/* Set the productivity mode's duration. */
	pmode_args->productivity_mode_duration = minutes;
	
	/* Start the productivity mode thread. */
	if (pthread_create(&pmode_thread_id, NULL, pmode_thread, (void *)pmode_args) != 0) {
		strcpy(error_message, "Productivity mode could not be activated.\n");
		return NULL;
	}
	
	strcpy(error_message, "");
	
	return &pmode_thread_id;
}

void *pmode_thread(void *input) {
	struct args *thread_args = (struct args *)input;
	int error;
	
	/*
	 * If a copy of the hosts file already exists,
	 * the last run did not end properly.
	 * This may be because of a force quit or a crash.
	 * In that case a clean-up is needed.
	 */
	if (access(HOSTS_FILE_COPY, F_OK) == 0) {
		error = unblock_domains();
		if (error != 0) {
			print_error_message("Blocked domains from last run could not be cleaned.", error);
			return NULL;
		}
	}
	
	/*
	 * Kill running processes that should be blocked
	 * and block the domains.
	 */
	kill_blocked_processes();
	error = block_domains();
	if (error != 0) {
		print_error_message("Blocking domains failed.", error);
		return NULL;
	}
	
	/* Initialize productivity mode arguments. */
	thread_args->productivity_mode_running = true;
	time(&thread_args->productivity_mode_start_time);
	
	/* Subscribe to the X11 window events. */
	Display *display = XOpenDisplay(NULL);
	XSelectInput(display, DefaultRootWindow(display), SubstructureNotifyMask);
	
	time_t now;
	XEvent e;

	while (thread_args->productivity_mode_running) {
		/* If there is a new X11 event... */
		if (XPending(display) > 0) {
			/* ...pop it from the event queue. */
			XNextEvent(display, &e);
			/* And if it was sent because a new window was created... */
			if (e.type == CreateNotify) {
				/* ...call the kill function. */
				kill_blocked_processes();
			}
		}

		/* Check if the productivity mode's duration is over. */
		time(&now);
		if (difftime(now, thread_args->productivity_mode_start_time) > thread_args->productivity_mode_duration) {
			thread_args->productivity_mode_running = false;
		}
	}
	
	/* Clean-up by unblocking the domains. */
	error = unblock_domains();
	if (error != 0) {
		print_error_message("Unblocking domains failed.", error);
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

bool is_browser_running() {
	FILE *command = popen("pgrep 'firefox|chrome'", "r");
	if (!command) {
		return false;
	}
	
	int ch = fgetc(command);
	fclose(command);
	
	return ch > 0;
}

void kill_blocked_processes() {
	clock_t begin = clock();

	/* 'pkill -15' terminates the process 'softly'. */
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
	
	fputs("127.0.0.1 youtube.com www.youtube.com\n", original_hosts_file);
	fputs("127.0.0.1 twitter.com www.twitter.com\n", original_hosts_file);
	fputs("127.0.0.1 facebook.com www.facebook.com\n\n", original_hosts_file);
	
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

void exit_productivity_mode() {
	// TODO check if user has permissions
	if (pmode_args != NULL) {
		pmode_args->productivity_mode_running = false;
	}
}

void show_status() {
	if (pmode_args != NULL && pmode_args->productivity_mode_running) {
		char time_str[50];
		strftime(time_str, 50, "%Y-%m-%d %H:%M", localtime(&pmode_args->productivity_mode_start_time));
		
		time_t now;
		time(&now);
		
		int passed_time = (int)difftime(now, pmode_args->productivity_mode_start_time);
		int remaining_time = pmode_args->productivity_mode_duration - passed_time;
		
	
		printf("Productivity mode running: yes\n");
		printf("Started at: %s\n", time_str);
		printf("Remaining time: %d minutes\n", remaining_time);
	} else {
		printf("Productivity mode running: no\n");
	}
}

