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
typedef struct Node Slist;  // Forward declaration

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
struct Node {
    pid_t pid;
    char input_string[30];
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

#endif