#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define HOSTS_FILE "/etc/hosts"
#define HOSTS_FILE_COPY "/etc/hosts_prosh_copy"
#define MAX_ITERATIONS 1000000

/**
 * Blocks the given domains so that they cannot
 * be accessed.
 * 
 * @return 0 if successful, otherwise errno.
 */
int block_domains(int start_index, int domain_count, char *domains[]) {
	if (access(HOSTS_FILE_COPY, F_OK) == 0) {
		return -1;
	}

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
	
	fclose(hosts_file_copy);
	
	for (int i = start_index; i < domain_count; i++) {
		fputs("127.0.0.1 ", original_hosts_file);
		fputs(domains[i], original_hosts_file);
		fputs(" www.", original_hosts_file);
		fputs(domains[i], original_hosts_file);
		fputs("\n", original_hosts_file);
	}
	
	fputs("\n", original_hosts_file);
	
	fclose(original_hosts_file);
	
	return 0;
}

/**
 * Unblocks domains that were blocked at the start
 * of the productivity mode.
 * 
 * @return 0 if successful, otherwise errno.
 */
int unblock_domains() {
	if (access(HOSTS_FILE_COPY, F_OK) != 0) {
		return -1;
	}

	if (remove(HOSTS_FILE) != 0) {
		return errno;
	}
	
	if (rename(HOSTS_FILE_COPY, HOSTS_FILE) != 0) {
		return errno;
	}
	
	return 0;
}

int main(int argc, char *argv[]) {
	if (getuid() != 0) {
		printf("This program needs to be executed by the root user.\n");
		return EXIT_FAILURE;
	}
	
	if (argc < 2) {
		printf("proshdom [block/unblock]\n");
		return EXIT_FAILURE;
	}
	
	int error;
	if (strcmp(argv[1], "block") == 0) {
		if (argc == 2) {
			printf("proshdom block [domain] [domain] [domain] [...]\n");
			return EXIT_FAILURE;
		} else {
			error = block_domains(2, argc, argv);
			if (error == -1) {
				printf("Domains are already blocked.\n");
			}	
		}
	} else if (strcmp(argv[1], "unblock") == 0) {
		error = unblock_domains();
		if (error == -1) {
			printf("Domains are already unblocked.\n");
		}
	} else {
		printf("proshdom [block/unblock]\n");
		return EXIT_FAILURE;
	}
	
	/* Returns 0 if successful, otherwise errno. */
	return error;
}
