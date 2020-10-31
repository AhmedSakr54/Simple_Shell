#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

#define BUFFER 255

void log_process(pid_t cpid) {
    time_t t;
    time(&t);
    FILE * fp;
    fp = fopen("logs.txt", "a+");
    fprintf(fp, "Child Process with ID: %d terminated at %s", cpid, ctime(&t));
    fclose(fp);
}

void signal_handler(int signal) {
    int status;
    pid_t cpid;
    while ((cpid = waitpid(-1, &status, WNOHANG)) > 0) {
        continue;
    }
    // sleep(1);
    log_process(cpid);
}

char * get_space_separated_string(char * temp, int n, int delimeter_ascii) {
    char * space_separated_string = (char *) malloc(sizeof(char) * n);
    int i = 1;
    int k = 0;
    while (temp[i] != delimeter_ascii) {
        space_separated_string[k++] = temp[i++];
    }
    space_separated_string[k] = '\0';
    return space_separated_string;
}


/**
 * Parses the command done by the user into an array of char *
 * @param : the command written into the shell
 * @return : the array with all the tokens resulting from delimiting the command with " " delimeter 
 */
char ** parse_command(char * command, int * size) {
    int n = 1;
    const char delimeter[2] = " ";
    // get the first token
    char * token = strtok(command, delimeter);
    // initialize the array of tokens 
    char ** parsed_command = (char **) malloc(sizeof(char*) * n);
    int i = 0;
    while(token != NULL) {
        // check if there is single quotes or double quotes
        // if true then directory or file name has a space in it
        if (token[0] == 39 || token[0] == 34) { 
            int delimeter_ascii;
            if (token[0] == 39)
                delimeter_ascii = 39; // ascii for single quotes (')
            else 
                delimeter_ascii = 34; // ascii for double quotes (")
            char* temp = (char *) malloc(sizeof(char) * BUFFER);
            strcpy(temp, token);
            // loop until the second end of the quotes
            while (token[strlen(token)-1] != delimeter_ascii) {
                // add the space back to the string 
                strcat(temp, " ");
                token = strtok(NULL, delimeter);
                // concatenate the other parts of the string that is between the quotes
                strcat(temp, token);
            } // at the end of the loop temp will contain the quotes and the space separated file name

            int temp_size = strlen(temp);
            // putting the space separated file or folder name into parsed_command without the quotes
            parsed_command[i++] = get_space_separated_string(temp, temp_size, delimeter_ascii);
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


void print_command_error_msg(char * erroneous_command) {
    printf("%s: command not found\n", erroneous_command);
}
void print_dir_error_msg(char * erroneous_dir) {
    printf("bash: cd: %s: No such file or directory\n", erroneous_dir);
}

void execute_child_process(char ** command_array, int size) {
    pid_t process_id;
    int background_flag = 0;
    //signal(SIGCHLD, log_process);
    process_id = fork();
    if (command_array[size-1] != NULL && strcmp(command_array[size-1],"&") == 0) {
        command_array[size-1] = NULL;
        background_flag = 1;
    }
    if (process_id >= 0) {
        if (process_id == 0) {
            int ret;
            ret = execvp(command_array[0], command_array);
            if (ret < 0) {
                print_command_error_msg(command_array[0]);
            }
            exit(0);
        }
        else {
            if (!background_flag) {
                int status;
                pid_t cpid;
                cpid = waitpid(process_id, &status, 0);
                // sleep(1);
                log_process(cpid);
            }
            else {
                signal(SIGCHLD, signal_handler);
            }
        }
    }
    else {
        printf("ERROR\n");
    }
}

int main() {
    char str[BUFFER] = "";
    int size = 0;
    while (1) {
        printf("Shell > ");
        fgets(str, BUFFER, stdin); 
        if (strcmp(str, "\n") == 0) 
            continue;
        str[strlen(str)-1] = '\0';
        if (strcmp(str, "exit") == 0)
            exit(0);
        char ** command_array = parse_command(str, &size);
        if (strcmp(command_array[0], "cd") == 0) {
            if (chdir(command_array[1]) != 0) {
                print_dir_error_msg(command_array[1]);
            }
        }
        else {
            execute_child_process(command_array, size);
        }
    }
    return 0;
}