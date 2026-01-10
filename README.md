MINISHELL PROJECT

An advanced UNIX-style minishell written in C that supports external command
execution, built-in commands, multi-pipe handling, and signal handling.

--------------------------------------------------
FEATURES
--------------------------------------------------
1. External command execution
   - Supports standard Linux commands such as ls, pwd, cat, etc.

2. Built-in commands
   - cd
   - pwd
   - exit

3. Pipe support
   - Single pipe support
     Example: ls | wc
   - Multiple pipe support
     Example: ls | grep .c | wc

4. Signal handling
   - Ctrl+C (SIGINT)
   - Ctrl+Z (SIGTSTP)
   - Shell does not terminate on signals

--------------------------------------------------
COMPILATION AND EXECUTION
--------------------------------------------------
Compile the project using GCC:

gcc *.c -o minishell

Run the shell:

./minishell

--------------------------------------------------
SAMPLE COMMANDS TO TEST
--------------------------------------------------
ls
ls -l
ls | wc
ls | grep c
ls | grep .c | wc
cat external_command.txt | wc
cat external_command.txt | grep main | wc -l

--------------------------------------------------
LEARNING OUTCOMES
--------------------------------------------------
This project demonstrates understanding of:

- fork() for process creation
- execvp() for program execution
- pipe() for inter-process communication
- dup2() for file descriptor redirection
- waitpid() for process synchronization
- UNIX signal handling (SIGINT, SIGTSTP)

--------------------------------------------------
AUTHOR
--------------------------------------------------
Name   : Siddharth Bhalerao
GitHub : https://github.com/siddharthbhalerao3529-gif

--------------------------------------------------
LICENSE
--------------------------------------------------
This project is developed for educational purposes.

