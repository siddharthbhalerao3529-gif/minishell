/*
name - siddharth bhalerao
date - 13-1-26
description - MiniShell (msh) is a minimal UNIX-style shell implemented in C.
              It supports external command execution, built-in commands, pipes, job control, and signal handling.
              The project demonstrates core Linux system programming concepts.
*/
#include "main.h"

char prompt[] = ANSI_COLOR_GREEN "minishell" ANSI_COLOR_RESET ":" ANSI_COLOR_BLUE "~" ANSI_COLOR_RESET "$ ";
char input_string[200];
char *ext_cmd[MAX_COMMAND];
char *external_commands[MAX_COMMAND];

Slist *head = NULL;
int status = 0;
pid_t pid = 0;

int main()
{
    // clear the terminal , system()
    system("clear");
    printf("\n");
    extract_external_commands(ext_cmd);
    for (int i = 0; i < MAX_COMMAND; i++)
    {
        external_commands[i] = ext_cmd[i];
    }
    scan_input(prompt, input_string);
    return 0;
}