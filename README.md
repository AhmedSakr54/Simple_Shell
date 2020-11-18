# Simple_Shell
- This program simulates the linux terminal.  
- It creates a child processes for executing execvp commands and when the child process terminates it logs the child id and the time of termination in a file called logs.txt.  
- the program handles foreground and background processes.  
- You can create a files or folders with spaces inbetween i.e mkdir 'new folder'

## Major Functions in the code:
### 1.	run_shell(char * str)
checks if the user just pressed enter or if the command was a change directory or if it was an exit command and if it wasn’t none of the previous then it calls the parse_command function.
parameter (char * str): the user input from the terminal
### 2.	char ** parse_command(char * command)
tokenizes the user input and divides it into a command and parameter i.e (ls -l -a will be in an array where the first element in that array is considered the command and the rest of the elements are the parameters) the resulting array will be returned.
parameter (char * command): the command written into the shell
### 3.	execute_child_process(char ** command_array)
creates a child process with the fork() function and then passes the command array as arguments to the execvp() function which will execute the command in the child process.
This function handles the foreground and background processes that the user might request.
paramemter (char ** command_array): This parameter is to be an argument for the execvp() function.

## Sample Runs:
![Screenshot (159)](https://user-images.githubusercontent.com/47868202/99543861-52857300-29bc-11eb-9725-446bd10e44d2.png)

![Screenshot (160)](https://user-images.githubusercontent.com/47868202/99543911-64ffac80-29bc-11eb-8112-9ba3fb58366f.png)

![Screenshot (161)](https://user-images.githubusercontent.com/47868202/99543916-6630d980-29bc-11eb-84c6-4d80538940eb.png)

![Screenshot (162)](https://user-images.githubusercontent.com/47868202/99543922-66c97000-29bc-11eb-8a7f-8f40169d0f82.png)

![Screenshot (163)](https://user-images.githubusercontent.com/47868202/99543925-68933380-29bc-11eb-92b8-069e75177e83.png)
