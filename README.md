MINI-SHELL (msh) PROJECT REQUIREMENTS

Description:
Implement a minimalistic shell, mini-shell (msh), as part of the Linux Internal module.

Objective:
The objective of this project is to understand and use Linux system calls related to:
- Process creation
- Signal handling
- Process synchronization
- Exit status handling
- Text parsing
- Job control

--------------------------------------------------
1. PROMPT REQUIREMENTS
--------------------------------------------------

1. Provide a prompt for the user to enter commands.
2. Display the default prompt as:
   msh>
3. Prompt should be customizable using environment variable PS1.
   - To change the prompt, user will enter:
     PS1=NEW_PROMPT
   - Whitespaces are NOT allowed around '='.
     Example (Valid):
     PS1=hello>
     Example (Invalid – treated as normal command):
     PS1 = hello>

--------------------------------------------------
2. COMMAND EXECUTION
--------------------------------------------------

1. User enters a command to execute.
2. If it is an external command:
   - Create a child process using fork().
   - Execute the command using exec family calls.
   - Parent process should wait for child completion.
   - After completion, msh prompt should be displayed.
3. If user presses ENTER without a command:
   - Prompt should be displayed again.

--------------------------------------------------
3. SPECIAL VARIABLES
--------------------------------------------------

1. Exit status of last command:
   - echo $? should print the exit status of the last executed command.

2. PID of msh:
   - echo $$ should print the PID of the msh process.

3. Shell name:
   - echo $SHELL should print the absolute path of msh executable.

--------------------------------------------------
4. SIGNAL HANDLING
--------------------------------------------------

Provide shortcuts to send signals to running programs.

1. Ctrl + C (SIGINT)
   - If a foreground process is running:
     Send SIGINT to the child process.
   - If no foreground process exists:
     Re-display msh prompt.

2. Ctrl + Z (SIGTSTP)
   - Stop the foreground process.
   - Parent should display the PID of the stopped child process.

--------------------------------------------------
5. BUILT-IN COMMANDS
--------------------------------------------------

1. exit
   - Terminates the msh program.

2. cd
   - Changes the current working directory.

3. pwd
   - Displays the current working directory.

--------------------------------------------------
6. BACKGROUND PROCESS / JOB CONTROL
--------------------------------------------------

1. Allow programs to run in background using '&'.
   Example:
   sleep 50 &

2. Implement job control commands:
   - bg
     * Moves a stopped process to background.
     * sleep 10 & is equivalent to:
       sleep 10
       Ctrl + Z
       bg

   - fg
     * Brings a background process to foreground.
     * fg without arguments brings last background process.
     * fg <pid> brings specified PID to foreground.

   - jobs
     * Displays all background process details.

3. After a background process ends:
   - Clean up the process using wait().
   - Display exit status of the terminated background process.
   - Use SIGCHLD signal handler if required.

4. User should be able to run multiple background processes simultaneously.

--------------------------------------------------
7. PIPE FUNCTIONALITY
--------------------------------------------------

1. Allow communication between multiple processes using pipes.
2. Pipes and child processes should be created dynamically based on command input.
3. Support multiple pipes in a single command.

Examples:
- ls | wc
- ls -l /dev | grep tty | wc -l

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

This project is developed for educational purposes only.
