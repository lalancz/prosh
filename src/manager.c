#define MAX_DOMAINS 100
#define MAX_DOMAIN_LEN 100

#define MAX_PROCESSES 100
#define MAX_PROCESS_LEN 100

char blocked_domains[MAX_DOMAINS][MAX_DOMAIN_LEN];
int blocked_domains_amount = 0;

char blocked_processes[MAX_PROCESSES][MAX_PROCESS_LEN];
int blocked_processes_amount = 0;

void add_blocked_domain(char *blocked_domain_string)
{
	if (strncmp("www.", blocked_domain_string, 4) == 0 || strncmp("http", blocked_domain_string, 4) == 0)
	{
		printf("domain names must not include their protocol or sub domain, proper usage example: youtube.com\n\n");
		return;
	}

	if (blocked_domains_amount == MAX_DOMAINS)
	{
		printf("max amount of domains reached\n\n");
		return;
	}

	if (strlen(blocked_domain_string) > MAX_DOMAIN_LEN)
	{
		printf("domain name longer than allowed\n\n");
		return;
	}

	int i;
	for (i = 0; i < MAX_DOMAINS; i++)
	{
		if (strcmp(blocked_domains[i], blocked_domain_string) == 0)
		{
			printf("domain already blocked\n\n");
			return;
		}
	}

	strcpy(blocked_domains[blocked_domains_amount], blocked_domain_string);
	blocked_domains_amount++;

	printf("domain name: %s added to block list\n\n", blocked_domain_string);
}

void remove_blocked_domain(char *blocked_domain_string)
{
	if (strncmp("www.", blocked_domain_string, 4) == 0 || strncmp("http", blocked_domain_string, 4) == 0)
	{
		printf("domain names must include their protocol or sub domain, proper usage example: youtube.com\n\n");
		return;
	}

	if (0 > blocked_domains_amount)
	{
		printf("no domains in block list\n\n");
		return;
	}

	int i;
	int pos = -1;
	for (i = 0; i < MAX_DOMAINS; i++)
	{
		if (strcmp(blocked_domains[i], blocked_domain_string) == 0)
		{
			pos = i;
			break;
		}
	}

	if (pos == -1)
	{
		printf("domain name: %s not found in block list\n\n", blocked_domain_string);
		return;
	}

	for (i = pos; i < MAX_DOMAINS - 1; i++)
	{
		strcpy(blocked_domains[i], blocked_domains[i + 1]);
	}

	blocked_domains_amount--;

	printf("domain name: %s removed from block list\n\n", blocked_domain_string);
}

void add_blocked_process(char *blocked_process_string)
{
	if (blocked_processes_amount == MAX_PROCESSES)
	{
		printf("max amount of processes reached\n\n");
		return;
	}

	if (strlen(blocked_process_string) > MAX_PROCESS_LEN)
	{
		printf("process name longer than allowed\n\n");
		return;
	}

	int i;
	for (i = 0; i < MAX_PROCESSES; i++)
	{
		if (strcmp(blocked_processes[i], blocked_process_string) == 0)
		{
			printf("process already blocked\n\n");
			return;
		}
	}

	strcpy(blocked_processes[blocked_processes_amount], blocked_process_string);
	blocked_processes_amount++;

	printf("process name: %s added to block list\n\n", blocked_process_string);
}

void remove_blocked_process(char *blocked_process_string)
{
	if (0 > blocked_processes_amount)
	{
		printf("no processes in block list\n\n");
		return;
	}

	int i;
	int pos = -1;	
	for (i = 0; i < MAX_PROCESSES; i++)
	{
		if (strcmp(blocked_processes[i], blocked_process_string) == 0)
		{
			pos = i;
			break;
		}
	}

	if (pos == -1)
	{
		printf("process name: %s not found in block list\n\n", blocked_process_string);
		return;
	}

	for (i = pos; i < MAX_PROCESSES - 1; i++)
	{
		strcpy(blocked_processes[i], blocked_processes[i + 1]);
	}

	blocked_processes_amount--;

	printf("process name: %s removed from block list\n\n", blocked_process_string);
}

void show_blocked_domains() 
{
	printf("\n");

	int i;
	for (i = 0; i < MAX_DOMAINS; i++)
	{
		if (strcmp(blocked_domains[i], "") != 0)
			printf("%s\n", blocked_domains[i]);
	}

	printf("\n");
}

void show_blocked_processes()
{
	printf("\n");

	int i;
	for (i = 0; i < MAX_PROCESSES; i++)
	{
		if (strcmp(blocked_processes[i], "") != 0)
			printf("%s\n", blocked_processes[i]);
	}

	printf("\n");
}