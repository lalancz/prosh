#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

int start_productivity_mode(int minutes);
int exit_productivity_mode();
int exit_productivity_mode_early();
void show_status();
void sig_handler(int signum);

char error_message[100];
int productivity_mode_running = 0;
time_t productivity_mode_start_time = 0;

int start_productivity_mode(int minutes) {
	if (productivity_mode_running == 1) {
		strcpy(error_message, "Productivity mode already on.\n");
		return 1;
	} else {
		productivity_mode_running = 1;
		time(&productivity_mode_start_time);
		strcpy(error_message, "");
		signal(SIGALRM, sig_handler);
		alarm(minutes * 60);
		// TODO enable restrictions
		return 0;
	}
}

void sig_handler(int signum) {
	int error = exit_productivity_mode();
	if (error == 1) {
		printf("%s", error_message);
	} else {
		printf("Productivity mode deactivated.\n");
	}
}

int exit_productivity_mode() {
	if (productivity_mode_running == 0) {
		strcpy(error_message, "Productivity mode not running.\n");
		return 1;
	} else {
		alarm(0); // Cancel the alarm.
		productivity_mode_running = 0;
		strcpy(error_message, "");
		// TODO disable restrictions
		return 0;
	}
}

int exit_productivity_mode_early() {
	// TODO check if user has permissions
	return exit_productivity_mode();
}

void show_status() {
	if (productivity_mode_running == 1) {
		char time_str[50];
		strftime(time_str, 50, "%Y-%m-%d %H:%M", localtime(&productivity_mode_start_time));
	
		printf("Productivity mode running: yes\n");
		printf("Started at: %s\n", time_str);
		printf("Remaining time: ?\n");
	} else {
		printf("Productivity mode running: no\n");
	}
}
