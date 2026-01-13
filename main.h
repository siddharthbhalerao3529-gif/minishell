#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

// ========== FORWARD DECLARATIONS ==========
typedef struct Node Slist; // Forward declaration

#define JOB_STOPPED 0
#define JOB_RUNNING 1
#define JOB_DONE 2

// ========== CONSTANTS ==========
#define BUILTIN 1
#define EXTERNAL 2
#define NO_COMMAND 3
#define MAX_LENGTH 200
#define MAX_COMMAND 152

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

// ========== STRUCTURE DEFINITION ==========
struct Node
{
    pid_t pid;
    char input_string[100];
    int state;
    struct Node *link;
};

// ========== EXTERN DECLARATIONS ==========
extern char prompt[];
extern char input_string[];
extern char *ext_cmd[];
extern char *external_commands[];
extern Slist *head;
extern int status;
extern pid_t pid;

// ========== FUNCTION PROTOTYPES ==========
void scan_input(char *prompt, char *input_string);
char *get_command(char *input_string);
int check_command_type(char *command);
void execute_internal_commands(char *input_string);
void execute_external_commands(char *input_string);
void signal_handler(int sig_num);
void extract_external_commands(char **external_commands);
void insert_at_first(Slist **head, pid_t pid, char *input_string);
void delete_element(Slist **head, pid_t pid);

void print_list(Slist **head);
void update_job_state(Slist *head, pid_t pid, int state);

Slist* get_job_by_number(Slist *head, int job_num);
int get_job_number(Slist *head, pid_t pid);
int parse_job_specifier(char *spec, Slist *head);
#endif