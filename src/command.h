/* Include guard to prevent code from being included twice. */
#ifndef COMMAND
#define COMMAND

pthread_t *start_productivity_mode(int minutes);
void show_status();
void *pmode_thread(void *input);
void kill_blocked_processes();
void print_error_message(char *message, int code);
int exit_productivity_mode();
int block_domains();
int unblock_domains();
bool is_browser_running();

#endif

