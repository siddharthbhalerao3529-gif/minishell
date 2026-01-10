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
        if (pid > 0)
        {
            insert_at_first(&head, pid, input_string);
            kill(pid, SIGTSTP);
        }
        else
        {
            // FIX: Just print newline, not prompt (main loop will print prompt)
            printf("\n");
            fflush(stdout);
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
                if (WIFEXITED(child_status) || WIFSIGNALED(child_status))
                {
                    delete_element(&head, child_pid);
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

    // Print initial prompt

    while (1) // Infinite loop to continuously process user input
    {
        if (strlen(input_string) != 0)
            memset(input_string, 0, strlen(input_string)); // Clear previous input if any

        printf("%s", prompt);
        fflush(stdout);

        scanf(" %[^\n]", input_string);

        if (strlen(input_string) == 0)
        {
            // Print prompt for empty input
            // printf("%s", prompt);
            // fflush(stdout);
            continue;
        }

        if (strncmp(input_string, "PS1=", 4) == 0)
        {
            if (*(input_string + 4) != ' ')
            {
                strcpy(prompt, input_string + 4);
                // Print prompt after PS1 change
                // printf("%s", prompt);
                // fflush(stdout);
            }
            else
            {
                printf("Invalid PS1\n");
                // fflush(stdout);
            }
            continue;
        }
        else
        {
            char *command = get_command(input_string);
            int type = check_command_type(command);

            // type is builtin
            if (type == BUILTIN)
            {
                execute_internal_commands(input_string);
                // FIX: Remove \n - builtins already print newline
                // printf("%s", prompt);
                // fflush(stdout);
            }
            // type is external OR pipe command
            else if (type == EXTERNAL || strchr(input_string, '|') != NULL)
            {
                pid = fork();

                if (pid > 0)
                {
                    int status = 0;
                    waitpid(pid, &status, WUNTRACED);
                    // Print prompt after waiting
                    // printf("%s", prompt);
                    // fflush(stdout);
                }
                else if (pid == 0)
                {
                    signal(SIGINT, SIG_DFL);
                    signal(SIGTSTP, SIG_DFL);
                    execute_external_commands(input_string);
                    exit(0);
                }
            }
            else
            {
                printf("Invalid command\n");
                // FIX: Remove \n - error already prints newline
                // printf("%s", prompt);
                // fflush(stdout);
            }
        }
    }
}