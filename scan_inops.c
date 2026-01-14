#include "main.h"

void signal_handler(int sig_num)
{
    if (sig_num == SIGINT)
    {
        if (pid == 0) // no foreground process
        {
            printf("\n%s", prompt);
            fflush(stdout);
        }
        else
        {
            // Send SIGINT to foreground child process
            kill(pid, SIGINT);
        }
    }
    else if (sig_num == SIGTSTP)
    {
        if (pid > 0) // We have a foreground process
        {
            kill(pid, SIGTSTP);
        }
        else
        {
            printf("\n%s", prompt);
            fflush(stdout);

            // Find and stop the most recent running background job
            Slist *temp = head;
            while (temp != NULL)
            {
                if (temp->state == JOB_RUNNING)
                {
                    // Try to stop this background job
                    if (kill(temp->pid, SIGTSTP) == 0)
                    {
                        // The job will be marked as stopped by SIGCHLD handler
                        break;
                    }
                }
                temp = temp->link;
            }
        }
    }
    else if (sig_num == SIGCHLD)
    {
        pid_t child_pid = 0;
        int child_status = 0;
        while ((child_pid = waitpid(-1, &child_status, WNOHANG | WUNTRACED)) > 0)
        {
            if (child_pid > 0)
            {
                int found = 0;
                Slist *temp = head;
                while (temp != NULL)
                {
                    if (temp->pid == child_pid)
                    {
                        found = 1;
                        if (WIFSTOPPED(child_status))
                        {
                            temp->state = JOB_STOPPED;
                            printf("\n[%d]+ Stopped\t%s\n", child_pid, temp->input_string);
                        }
                        else if (WIFEXITED(child_status) || WIFSIGNALED(child_status))
                        {
                            if (child_pid != pid)
                            {
                                delete_element(&head, child_pid);
                            }
                        }
                        break;
                    }
                    temp = temp->link;
                }
                if (!found && WIFSTOPPED(child_status))
                {
                    insert_at_first(&head, child_pid, input_string);
                    update_job_state(head, child_pid, JOB_STOPPED);
                    printf("\n[%d]+ Stopped\t%s\n", child_pid, input_string);
                }
                if (child_pid == pid)
                {
                    pid = 0;
                }
            }
        }
    }
}

void scan_input(char *prompt, char *input_string)
{
    signal(SIGINT, signal_handler);
    signal(SIGTSTP, signal_handler);
    signal(SIGCHLD, signal_handler);

    while (1)
    {
        if (strlen(input_string) != 0)
            memset(input_string, 0, strlen(input_string));

        printf("%s", prompt);
        fflush(stdout);

        scanf(" %199[^\n]", input_string);

        if (strlen(input_string) == 0)
        {
            continue;
        }

        if (strncmp(input_string, "PS1=", 4) == 0)
        {
            if (*(input_string + 4) != ' ')
            {
                char new_prompt[100];
                char *user_prompt = input_string + 4;

                // Format: GREEN user_prompt : BLUE ~ $ RESET
                snprintf(new_prompt, sizeof(new_prompt),
                         ANSI_COLOR_GREEN "%s" ANSI_COLOR_RESET ":" ANSI_COLOR_BLUE "~" ANSI_COLOR_RESET "$ ",
                         user_prompt);

                strcpy(prompt, new_prompt);
            }
            else
            {
                printf("Invalid PS1\n");
            }
            continue;
        }
        else
        {
            char *command = get_command(input_string);
            int type = check_command_type(command);

            if (type == BUILTIN)
            {
                execute_internal_commands(input_string);
            }
            else if (type == EXTERNAL || strchr(input_string, '|') != NULL)
            {
                int run_in_background = 0;
                int input_len = strlen(input_string);

                while (input_len > 0 && input_string[input_len - 1] == ' ')
                    input_len--;

                if (input_len > 0 && input_string[input_len - 1] == '&')
                {
                    run_in_background = 1;
                    input_string[input_len - 1] = '\0';
                    while (input_len > 1 && input_string[input_len - 2] == ' ')
                    {
                        input_string[input_len - 2] = '\0';
                        input_len--;
                    }
                }

                pid = fork();

                if (pid > 0)
                {
                    if (run_in_background)
                    {
                        // Background job
                        insert_at_first(&head, pid, input_string);
                        update_job_state(head, pid, JOB_RUNNING);
                        printf("[%d] %d\n", get_job_number(head, pid), pid);
                        pid = 0; // No foreground process
                    }
                    else
                    {
                        // Foreground job
                        int child_status = 0;
                        waitpid(pid, &child_status, WUNTRACED);

                        if (WIFSTOPPED(child_status))
                        {
                            insert_at_first(&head, pid, input_string);
                            update_job_state(head, pid, JOB_STOPPED);
                            printf("\n[%d]+ Stopped\t%s\n", pid, input_string);
                        }
                        pid = 0;
                    }
                }
                else if (pid == 0)
                {
                    // Child process
                    if (run_in_background)
                    {
                        // Background job: Ignore SIGTSTP so Ctrl+Z doesn't affect it
                        signal(SIGTSTP, SIG_IGN);
                        signal(SIGINT, SIG_IGN);
                    }
                    else
                    {
                        // Foreground job: Use default signal handling
                        signal(SIGINT, SIG_DFL);
                        signal(SIGTSTP, SIG_DFL);
                    }

                    execute_external_commands(input_string);
                    exit(0);
                }
            }
            else if (type == NO_COMMAND)
            {
                char *extn_cmd = get_command(input_string);
                if (strcmp(extn_cmd, "jobs") == 0)
                {
                    print_list(&head);
                }
                else if (strcmp(extn_cmd, "fg") == 0)
                {
                    char *arg = input_string + 2;
                    while (*arg == ' ')
                        arg++;

                    int job_num = -1;

                    if (*arg == '\0')
                    {
                        Slist *temp = head;
                        while (temp != NULL)
                        {
                            if (temp->state == JOB_STOPPED)
                            {
                                job_num = get_job_number(head, temp->pid);
                                break;
                            }
                            temp = temp->link;
                        }

                        if (job_num == -1)
                        {
                            printf("fg: no current job\n");
                            continue;
                        }
                    }
                    else
                    {
                        job_num = parse_job_specifier(arg, head);
                        if (job_num == -1)
                        {
                            printf("fg: %s: no such job\n", arg);
                            continue;
                        }
                    }

                    Slist *job = get_job_by_number(head, job_num);
                    if (job == NULL)
                    {
                        printf("fg: job %d not found\n", job_num);
                        continue;
                    }

                    if (job->state != JOB_STOPPED)
                    {
                        printf("fg: job %d is not stopped\n", job_num);
                        continue;
                    }

                    if (kill(job->pid, 0) == -1)
                    {
                        printf("fg: job %d no longer exists\n", job_num);
                        delete_element(&head, job->pid);
                        continue;
                    }

                    printf("Bringing job [%d] to foreground: %s\n", job_num, job->input_string);

                    job->state = JOB_RUNNING;
                    pid = job->pid;

                    if (kill(job->pid, SIGCONT) == -1)
                    {
                        perror("fg");
                        pid = 0;
                        job->state = JOB_STOPPED;
                    }
                    else
                    {
                        int wait_status;
                        waitpid(job->pid, &wait_status, WUNTRACED);

                        if (WIFSTOPPED(wait_status))
                        {
                            job->state = JOB_STOPPED;
                        }
                        else if (WIFEXITED(wait_status) || WIFSIGNALED(wait_status))
                        {
                            delete_element(&head, job->pid);
                        }
                        pid = 0;
                    }
                }
                else if (strcmp(extn_cmd, "bg") == 0)
                {
                    if (head != NULL)
                    {
                        Slist *temp = head;
                        Slist *stopped_job = NULL;
                        while (temp != NULL)
                        {
                            if (temp->state == JOB_STOPPED)
                            {
                                stopped_job = temp;
                                break;
                            }
                            temp = temp->link;
                        }

                        if (stopped_job != NULL)
                        {
                            if (kill(stopped_job->pid, 0) == -1)
                            {
                                printf("bg: job [%d] no longer exists\n", stopped_job->pid);
                                delete_element(&head, stopped_job->pid);
                                continue;
                            }

                            pid_t bpid = stopped_job->pid;

                            printf("Continuing job [%d] in background: %s\n", bpid, stopped_job->input_string);

                            if (kill(bpid, SIGCONT) == -1)
                            {
                                perror("bg: kill");
                            }
                            else
                            {
                                stopped_job->state = JOB_RUNNING;
                            }
                        }
                        else
                        {
                            printf("bg: no stopped jobs\n");
                        }
                    }
                    else
                    {
                        printf("bg: no current job\n");
                    }
                }
                else
                    printf("Invalid command\n");
            }
        }
    }
}