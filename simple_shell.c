#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

#define BUFFER 255

char PATH_OF_LOGS_FILE[BUFFER]; // so that the logs.txt file will be in the same place no matter the current working directory


void print_command_error_msg(char * erroneous_command) {
    printf("%s: command not found\n", erroneous_command);
}
void print_dir_error_msg(char * erroneous_dir) {
    printf("bash: cd: %s: No such file or directory\n", erroneous_dir);
}
void print_command_foramating_error_msg() {
    printf("inconsistent double of single quotes....\n");
}

/**
 * Logs the creation or termination of the parent process by writing the process ID with system time next to it
 * @param (int start_process) : when 1 means log the start of the parent process when 0 log the termination of the process
 **/ 
void log_parent_process(int start_process) {
    time_t t;
    time(&t);
    FILE * fp;
    fp = fopen(PATH_OF_LOGS_FILE, "a+");
    if (start_process) {
        fprintf(fp, "===================================================================\n");
        fprintf(fp, "Parent Process with ID: %d started at %s\n", getpid(), ctime(&t));
    }
    else {
        fprintf(fp, "\nParent Process with ID: %d ended at %s", getpid(), ctime(&t));
    }
    fclose(fp);
}

/**
 * Logs the termination of child process by writing the process ID with system time next to it
 * @param (pid_t cpid) : the ID of the child process to be logged for termination
 **/
void log_process(pid_t cpid) {
    if (cpid <= 0) 
        return;
    time_t t;
    time(&t);
    FILE * fp;
    fp = fopen(PATH_OF_LOGS_FILE, "a+");
    fprintf(fp, "Child Process with ID: %d terminated at %s", cpid, ctime(&t));
    fclose(fp);
}

/**
 * Executes after the background child process has terminated
 **/ 
void signal_handler(int signal) {
    int status;
    pid_t cpid;
    // to get the process ID of the child
    cpid = waitpid(-1, &status, WNOHANG);
    while (waitpid(-1, &status, WNOHANG) > 0) {
        continue;
    }
    log_process(cpid);
}

/**
 * Removes quotes from the arg string 'Lab 1' -> Lab 1 or """Lab 1""" -> Lab 1
 * @param (char * quoted_string) : a string that is between quotes (single or double)
 * @param (int size_of_quoted_string) : the size of the string including the quotes
 * @param (int delimeter_ascii) : ascii for single or double quotes possible values are (39 or 34 respectivly)
 * @return : a string with the quotes removed
 **/
char * remove_quotes_from_string(char * quoted_string, int size_of_quoted_string, int delimeter_ascii) {
    char * non_quoted_string = (char *) malloc(sizeof(char) * size_of_quoted_string);
    // if we are in this function then the start of the actual text cannot be from index 0 so I try from index 1
    int index_of_actual_text = 1;
    // to handle cases where the user writes something like ''Lab 1'' 
    // so index_of_actual_text=2 when getting out of the loop
    while (quoted_string[index_of_actual_text] == delimeter_ascii) {
        index_of_actual_text++;
    }
    int k = 0;
    // filling non_quoted_string with the actual text with no quotes
    while (quoted_string[index_of_actual_text] != delimeter_ascii) {
        non_quoted_string[k++] = quoted_string[index_of_actual_text++];
    }
    non_quoted_string[k] = '\0';
    return non_quoted_string;
}

/**
 * Parses the command done by the user into an array of char *
 * @param (char * command) : the command written into the shell
 * @param (int * size) : a pointer to the size of the written command
 * @return : the array with all the tokens resulting from delimiting the command with " " delimeter 
 */
char ** parse_command(char * command, int * size) {
    int n = 1;
    char end_char = command[strlen(command) - 1];
    const char delimeter[2] = " ";
    // get the first token
    char * token = strtok(command, delimeter);
    // initialize the array of tokens 
    char ** parsed_command = (char **) malloc(sizeof(char*) * n);
    int i = 0;
    while(token != NULL) {
        // if true then directory or file name is between quotes
        if (token[0] == 39 || token[0] == 34) { // 39 ascii for single quote (') and 34 ascii for double quote (")
            int delimeter_ascii = token[0];
            
            char* temp = (char *) malloc(sizeof(char) * BUFFER);
            strcpy(temp, token);
            if (token[strlen(token)-1] != delimeter_ascii) {
                // loop until the second end of the quotes
                while (token[strlen(token)-1] != delimeter_ascii) {
                    // add the space back to the string 
                    strcat(temp, " ");
                    token = strtok(NULL, delimeter);
                    if (token == NULL) {
                        print_command_foramating_error_msg();
                        return NULL;
                    }
                    // concatenate the other parts of the string that is between the quotes
                    strcat(temp, token);
                } // at the end of the loop temp will contain the quotes and the space separated file name
            }
            int temp_size = strlen(temp);
            // putting the space separated file or folder name into parsed_command without the quotes
            parsed_command[i++] = remove_quotes_from_string(temp, temp_size, delimeter_ascii);
            free(temp);
        }
        else { // normal command with no quotes
            parsed_command[i++] = token;
        }
        token = strtok(NULL, delimeter);
        n++;
        // to be able to add more tokens into parsed_command
        parsed_command = (char **) realloc(parsed_command, sizeof(char *) * n);
    }
    // last element in parsed_command must be NULL for the execvp function
    parsed_command[n-1] = NULL;
    *size = n-1;
    return parsed_command;
}

/**
 * Creates a child process via the fork() function and executes it with the execvp() function
 * @param (char ** command_array) : This parameter is to be an argument for the execvp() function
 * @param (int size) : the size of the command array
 **/
void execute_child_process(char ** command_array, int size) {
    pid_t process_id;

    // when 0 means foreground process
    // when 1 means background process
    int background_flag = 0;
    // creating the child process and getting it's ID
    process_id = fork();
    // checking for the background operator & and setting backgorund_flag accordingly
    if (command_array[size-1] != NULL && strcmp(command_array[size-1],"&") == 0) {
        // removing the & from the array to be able to put it into the execvp() function
        // without problems
        command_array[size-1] = NULL;
        background_flag = 1;
    }
    if (process_id >= 0) { // no error occured in creating the child process
        if (process_id == 0) { // executing the child process
            int ret;
            ret = execvp(command_array[0], command_array);
            if (ret < 0) { // when true means there was an error in writing the initial command 
                print_command_error_msg(command_array[0]);
            }
            exit(0);
        }
        else { // executing the parent process
            if (!background_flag) { // foreground child process
                int status;
                pid_t cpid;
                // parent process waiting for the child process to terminate
                cpid = waitpid(process_id, &status, 0);
                // parent process logging the termination of the child process
                log_process(cpid);
            }
            else { // background child process
                // the parent process is waiting for a signal that the child process has terminated to
                // execute the signal_handler function
                signal(SIGCHLD, signal_handler);
            }
        }
    }
    else { // an error occured in creating the child process
        printf("ERROR\n");
    }
}

/**
 * pareses then runs what the user inputed into str
 * @param (char * str) : the user input from the terminal
 **/
void run_shell(char * str) {
    // checking if the user has just pressed enter before typing anything
    if (strcmp(str, "\n") == 0) 
        return;
    int size = 0;
    // using the fgets method in getting string input will end the string with a \n character
    // this is removing this \n character
    str[strlen(str)-1] = '\0';
    // terminating the process when the user types exit
    if (strcmp(str, "exit") == 0) {
        log_parent_process(0); // start_process = 0
        exit(0);
    }
    char ** command_array = parse_command(str, &size);
    if (command_array == NULL)
        return;
    // if the command was a change directory a child process will not be created
    // because the execvp doesn't execute cd command
    // using chdir() function instead
    // subsequently no termination will be logged in the logs.txt file because no process
    // was created to be logged
    if (strcmp(command_array[0], "cd") == 0) {
        if (command_array[1] == NULL) {
            chdir(getenv("HOME"));
        }
        else if (chdir(command_array[1]) != 0) {
            print_dir_error_msg(command_array[1]);
        }
    }
    else {
        execute_child_process(command_array, size);
    }
}

int main() {
    char str[BUFFER] = "";
    getcwd(PATH_OF_LOGS_FILE, BUFFER);
    strcat(PATH_OF_LOGS_FILE, "/logs.txt");
    log_parent_process(1);
    while (1) {
        printf("Shell > ");
        fgets(str, BUFFER, stdin);
        run_shell(str);
    }
    return 0;
}