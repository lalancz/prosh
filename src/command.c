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

pthread_t pmode_thread_id;

struct args {
	bool productivity_mode_running;
	time_t productivity_mode_start_time;
};

struct args pma = {false, (time_t)0};
struct args *pmode_args = &pma;

pthread_t *start_productivity_mode(char *error_message) {
	/*
	 * pmode_args is null on the first time the productivity mode is started.
	 * In that case the memory needs to be allocated.
	 */
	if (!pmode_args) {
		pmode_args = (struct args *) malloc(sizeof(struct args));
	}
	
	/* Do not start the productivity mode if it is already running. */
	if (pmode_args->productivity_mode_running) {
		strcpy(error_message, "Productivity mode already on.");
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
		strcpy(error_message, "Please close all browsers before starting the productivity mode.");
		return NULL;
	}
	
	/* Do not start the productivity mode if there is no hosts file. */
	if (access(HOSTS_FILE, F_OK) != 0) {
		strcpy(error_message, "Your hosts file ('etc/hosts') does not exist. It is needed to run the productivity mode.");
		return NULL;
	}
	
	/*
	 * If a copy of the hosts file already exists,
	 * the last run did not end properly.
	 * This may be because of a force quit or a crash.
	 * In that case a clean-up is needed.
	 */
	if (access(HOSTS_FILE_COPY, F_OK) == 0) {
		if (!unblock_domains()) {
			strcpy(error_message, "Blocked domains from last run could not be cleaned.");
			return NULL;
		}
	}
	
	/*
	 * Block the domains. This function uses
	 * root permissions with sudo.
	 */
	if (!block_domains()) {
		strcpy(error_message, "Blocking domains failed.");
		return NULL;
	}
	
	/*
	 * Reset the sudo timestamp.
	 * Thus the user has to enter the passwort again
	 * when executing the next sudo command.
	 */
	if (!reset_sudo_timestamp()) {
		strcpy(error_message, "Sudo timestamp could not be reset.");
		return NULL;
	}
	
	/*
	 * Kill all processes that will be blocked
	 * but are already running.
	 */
	kill_blocked_processes();
	
	/* Start the productivity mode thread. */
	if (pthread_create(&pmode_thread_id, NULL, pmode_thread, (void *)pmode_args) != 0) {
		strcpy(error_message, "Productivity mode could not be activated.");
		return NULL;
	}
	
	strcpy(error_message, "");
	
	return &pmode_thread_id;
}

void *pmode_thread(void *input) {
	struct args *thread_args = (struct args *)input;
	
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
	/* 'pkill -15' terminates the process 'softly'. */
	system("pkill -15 mines");
	system("pkill -15 mahjongg");
}

void exit_productivity_mode(char *error_message) {
	if (pmode_args == NULL || !pmode_args->productivity_mode_running) {
		strcpy(error_message, "The productivity mode is not running.");
	} else {
		pmode_args->productivity_mode_running = false;
		
		/* Clean-up by unblocking the domains. */
		if (unblock_domains()) {
			strcpy(error_message, "");
		} else {
			strcpy(error_message, "The productivity mode was ended but unblocking the domains failed.");
		}
		
		/* Force the user to enter their password again next time. */
		reset_sudo_timestamp();
	}
}

void show_status() {
	if (pmode_args != NULL && pmode_args->productivity_mode_running) {
		char time_str[50];
		strftime(time_str, 50, "%Y-%m-%d %H:%M", localtime(&pmode_args->productivity_mode_start_time));
	
		printf("Productivity mode running: yes\n");
		printf("Started at: %s\n", time_str);
	} else {
		printf("Productivity mode running: no\n");
	}
}

bool block_domains() {
	/*
	 * Blocking domains needs root permissions.
	 * Because we do not want to run the whole shell
	 * with sudo we moved the functions to block
	 * and unblock the domains into a separate
	 * executable that we call as super user.
	 */
	return system("sudo ./proshdom block") == 0;
}

bool unblock_domains() {
	/* Unblocking domains needs root permissions. */
	return system("sudo ./proshdom unblock") == 0;
}

bool reset_sudo_timestamp() {
	return system("sudo -k") != -1;
}

