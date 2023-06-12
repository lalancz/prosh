/* Include guard to prevent code from being included twice. */
#ifndef COMMAND
#define COMMAND

/*
 * Starts the productivity mode by creating a new thread.
 *
 * If the thread was created successfully, its thread ID
 * is returned. Otherwise, NULL is returned and error_message
 * is initialized with an explanation.
 *
 * @param minutes The productivity mode duration in minutes
 * @param error_message An explanation of the occured error
 * @return The thread id if successful, otherwise NULL
 */
pthread_t *start_productivity_mode(int minutes, char *error_message);

/*
 * Stops the running productivity mode.
 */
void exit_productivity_mode();

/*
 * Prints information about the productivity mode
 * to the console.
 */
void show_status();

/*
 * Runs the main productivity mode loop.
 *
 * @param input The args struct
 */
void *pmode_thread(void *input);

/*
 * Terminates all processes that are not allowed to run
 * while the productivity mode is running.
 */
void kill_blocked_processes();

/**
 * Checks if a browser application is open.
 * 
 * @return True if a browser is running, otherwise false
 */
bool is_browser_running();

/**
 * Blocks domains that should not be accessed
 * while the productivity mode is running
 * by calling 'sudo ./proshdom block'.
 * 
 * @return True if successful, otherwise false.
 */
bool block_domains();

/**
 * Unblocks domains that were blocked at the start
 * of the productivity mode by calling
 * 'sudo ./proshdom unblock'.
 * 
 * @return True if successful, otherwise false
 */
bool unblock_domains();

#endif

