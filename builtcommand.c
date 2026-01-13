#include "main.h"

char *builtins[] = {"echo", "printf", "read", "cd", "pwd", "pushd", "popd", "dirs", "let", "eval",
					"set", "unset", "export", "declare", "typeset", "readonly", "getopts", "source",
					"exit", "exec", "shopt", "caller", "true", "type", "hash", "bind", "help", NULL};

static char cmd[100];

char *get_command(char *command)
{
	int i = 0;
	memset(cmd, 0, sizeof(cmd));
	while (command[i] != '\0')
	{
		if (command[i] == ' ')
		{
			cmd[i] = '\0';
			return cmd;
		}
		if (i >= 99)
			break;
		cmd[i] = command[i];
		i++;
	}
	cmd[i] = '\0';
	return cmd;
}

int check_command_type(char *command)
{
	// builtin command check
	for (int i = 0; builtins[i] != NULL; i++)
	{
		if (strcmp(command, builtins[i]) == 0)
			return BUILTIN;
	}

	// external command check
	for (int i = 0; external_commands[i] != NULL; i++)
	{
		if (strcmp(command, external_commands[i]) == 0)
			return EXTERNAL;
	}
	// no command
	return NO_COMMAND;
}

void extract_external_commands(char **external_commands)
{
	int fd = open("external_command.txt", O_RDONLY);
	if (fd == -1)
	{
		perror("open");
		exit(1);
	}

	char str[100];
	int index = 0;
	int pos = 0;
	char ch;

	// printf("Loading external commands...\n");

	while (read(fd, &ch, 1) > 0 && index < MAX_COMMAND - 1)
	{
		if (ch != '\n' && ch != '\r')
		{
			str[pos++] = ch;
		}
		else if (pos > 0)
		{
			str[pos] = '\0';
			external_commands[index] = malloc(strlen(str) + 1);
			if (external_commands[index] == NULL)
			{
				perror("malloc");
				exit(1);
			}
			strcpy(external_commands[index], str);
			index++;
			pos = 0;
		}
	}

	// load the last command
	if (pos > 0 && index < MAX_COMMAND - 1)
	{
		str[pos] = '\0';
		external_commands[index] = malloc(strlen(str) + 1);
		if (external_commands[index] == NULL)
		{
			perror("malloc");
			exit(1);
		}
		strcpy(external_commands[index], str);
		index++;
	}

	// add NULL to the end of the array
	external_commands[index] = NULL;
	close(fd);
	// printf("Loaded %d external commands\n", index);
}

void execute_internal_commands(char *input_string)
{
	if (strcmp(input_string, "exit") == 0)
	{
		exit(0);
	}
	else if (strcmp(input_string, "pwd") == 0)
	{
		char *pwd = getcwd(NULL, 0);
		if (pwd == NULL)
		{
			perror("pwd");
			return;
		}
		printf(ANSI_COLOR_CYAN "%s" ANSI_COLOR_RESET "\n", pwd);
		free(pwd);
	}
	else if (strncmp(input_string, "cd ", 3) == 0)
	{
		char *path = input_string + 3;
		if (chdir(path) != 0)
		{
			printf(ANSI_COLOR_RED "cd: " ANSI_COLOR_RESET);
			perror(path);
		}
	}
	else if (strstr(input_string, "echo $$") != NULL)
	{
		printf("%d\n", getpid());
	}
	else if (strstr(input_string, "echo $SHELL") != NULL)
	{
		char *shell_path = getenv("SHELL");
		if (shell_path != NULL)
			printf("%s\n", shell_path);
		else
			printf("\n");
	}
	else if (strstr(input_string, "echo $?") != NULL)
	{
		printf("%d \n", WEXITSTATUS(status));
	}
	else
	{
		printf(ANSI_COLOR_YELLOW "Built-in command '%s' not yet implemented" ANSI_COLOR_RESET "\n", input_string);
	}
}

void execute_external_commands(char *input_string)
{
	char input_copy[200];
	strncpy(input_copy, input_string, sizeof(input_copy) - 1);
	input_copy[sizeof(input_copy) - 1] = '\0';

	char *cmds[MAX_COMMAND];
	int index = 0;
	char *token = strtok(input_copy, " ");
	while (token != NULL)
	{
		cmds[index++] = token;
		token = strtok(NULL, " ");
	}
	cmds[index] = NULL;

	int pipe_found = 0;
	for (int i = 0; cmds[i] != NULL; i++)
	{
		if (strcmp(cmds[i], "|") == 0)
		{
			pipe_found++;
		}
	}

	if (!pipe_found)
	{
		// Direct execvp - parent already forked
		execvp(cmds[0], cmds);
		perror("execvp");
		exit(1);
	}
	else
	{
		// implement pipe handling
		int pipefd[pipe_found][2];
		for (int i = 0; i < pipe_found; i++)
		{
			if (pipe(pipefd[i]) == -1)
			{
				perror("pipe");
				exit(1);
			}
		}

		int cmd_start = 0;

		for (int i = 0; i <= pipe_found; i++)
		{
			char *argv[MAX_COMMAND];
			int arg_index = 0;
			while (cmds[cmd_start] && strcmp(cmds[cmd_start], "|") != 0)
			{
				argv[arg_index++] = cmds[cmd_start++];
			}
			argv[arg_index] = NULL;

			cmd_start++;
			pid_t process = fork();
			if (process == 0)
			{
				if (i > 0)
					dup2(pipefd[i - 1][0], 0);

				if (i < pipe_found)
					dup2(pipefd[i][1], 1);

				for (int j = 0; j < pipe_found; j++)
				{
					close(pipefd[j][0]);
					close(pipefd[j][1]);
				}

				execvp(argv[0], argv);
				perror("execvp failed");
				exit(1);
			}
		}

		// Parent closes all pipes
		for (int i = 0; i < pipe_found; i++)
		{
			close(pipefd[i][0]);
			close(pipefd[i][1]);
		}

		// Wait for all child processes
		for (int i = 0; i <= pipe_found; i++)
		{
			wait(NULL);
		}

		return;
	}
}

void insert_at_first(Slist **head, pid_t pid, char *input_string)
{
	Slist *new = malloc(sizeof(Slist));
	if (new == NULL)
	{
		printf("Error: malloc failed\n");
		return;
	}
	new->pid = pid;
	new->state = JOB_RUNNING;

	strcpy(new->input_string, input_string);
	int l = strlen(input_string);
	new->input_string[l] = '\0';

	if (*head == NULL)
	{
		*head = new;
		new->link = NULL;
		return;
	}

	new->link = *head;
	*head = new;
}

void delete_element(Slist **head, pid_t pid)
{
	if (head == NULL || *head == NULL)
		return;

	Slist *temp = *head;
	Slist *prev = NULL;

	// Case 1: deleting head node
	if (temp->pid == pid)
	{
		*head = temp->link;
		free(temp);
		return;
	}

	// Case 2: deleting non-head node
	while (temp != NULL && temp->pid != pid)
	{
		prev = temp;
		temp = temp->link;
	}

	if (temp != NULL)
	{
		prev->link = temp->link;
		free(temp);
	}
}
void print_list(Slist **head)
{
	if (head == NULL || *head == NULL) // FIXED: Check *head, not head
	{
		printf("No jobs running\n");
		return;
	}

	// Clean up completed jobs first
	Slist *temp = *head;
	Slist *prev = NULL;

	while (temp != NULL)
	{
		// Only remove if state is JOB_DONE
		if (temp->state == JOB_DONE)
		{
			// Process is done, remove it
			Slist *to_free = temp;

			if (prev == NULL)
			{
				*head = temp->link;
				temp = *head; // temp moves to new head
							  // prev remains NULL for new head
			}
			else
			{
				prev->link = temp->link;
				temp = temp->link; // temp moves to next node
								   // prev stays the same (points to node before removed one)
			}

			free(to_free);
		}
		else
		{
			// For RUNNING or STOPPED jobs, just check if they're really alive
			if (temp->state == JOB_RUNNING && kill(temp->pid, 0) == -1)
			{
				// Running job is dead, mark as done
				temp->state = JOB_DONE;
			}

			prev = temp;
			temp = temp->link;
		}
	}

	// Rest of the function remains the same...
	// Now print the list
	temp = *head;
	int job_num = 1;

	if (temp == NULL)
	{
		printf("No jobs running\n");
		return;
	}

	printf("[Job#]\tPID\tState\tCommand\n");
	printf("------\t---\t-----\t-------\n");

	while (temp != NULL)
	{
		const char *state_str = "Unknown";
		if (temp->state == JOB_STOPPED)
			state_str = "Stopped";
		else if (temp->state == JOB_RUNNING)
			state_str = "Running";
		else if (temp->state == JOB_DONE)
			state_str = "Done";

		printf("[%d]\t%d\t%s\t%s\n", job_num++, temp->pid, state_str, temp->input_string);
		temp = temp->link;
	}
}
void update_job_state(Slist *head, pid_t pid, int state)
{
	Slist *temp = head;
	while (temp != NULL)
	{
		if (temp->pid == pid)
		{
			temp->state = state;
			break;
		}
		temp = temp->link;
	}
}

Slist *get_job_by_number(Slist *head, int job_num)
{
	Slist *temp = head;
	int count = 1;

	while (temp != NULL)
	{
		if (count == job_num)
			return temp;
		temp = temp->link;
		count++;
	}
	return NULL;
}

// Get job number by PID (1-based)
int get_job_number(Slist *head, pid_t pid)
{
	Slist *temp = head;
	int count = 1;

	while (temp != NULL)
	{
		if (temp->pid == pid)
			return count;
		temp = temp->link;
		count++;
	}
	return 0;
}

// Parse job specifier (returns job number or -1 for errors)
int parse_job_specifier(char *spec, Slist *head)
{
	if (spec == NULL || spec[0] != '%')
		return -1;

	char *num_str = spec + 1;

	// %+ or %% for current job (most recent)
	if (strcmp(num_str, "+") == 0 || strcmp(num_str, "%") == 0)
	{
		if (head != NULL)
			return 1; // Most recent is always job #1
		return -1;
	}

	// %- for previous job
	if (strcmp(num_str, "-") == 0)
	{
		if (head != NULL && head->link != NULL)
			return 2; // Second job is previous
		return -1;
	}

	// %n for job number n
	char *endptr;
	long job_num = strtol(num_str, &endptr, 10);

	if (endptr == num_str || *endptr != '\0')
		return -1; // Not a valid number

	// Check if job exists
	if (get_job_by_number(head, job_num) == NULL)
		return -1;

	return (int)job_num;
}