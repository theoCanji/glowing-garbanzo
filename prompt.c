#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define BUFFER_SIZE 1024
#define COMMAND_SIZE 1024

//function to display promt for the shell
void displayPrompt(){
    char cwd[BUFFER_SIZE];
    if(getcwd(cwd, sizeof(cwd)) != NULL){
        printf("%s> ", cwd);
    } 
    else {
        perror("getcwd() error");
    }
}

// get user input
void getInput(char *command){

    if (fgets(command, COMMAND_SIZE, stdin ) == NULL){
        perror("fgets() error");
    }
}

// parse the input using strtok
void parseInput(char* command, char **args){

    command[strcspn(command, "\n")] = 0; //remove newlien


    char* token = strtok(command, " ");
    int i = 0; 
    while (token != NULL){
        args[i] = token;
        token = strtok(NULL, " "); //turns out this function just picks up where the last call left off
        i++; 
    }
    args[i] = NULL; //last index is set to NULL
}

//execute the command (exit and cd). non-zero value is returned if the command is not executed successfully
int execute(char **args){
    if (strcmp(args[0], "exit") == 0 ){
        printf("Goodbye!\n");
        return 1; 
    }
    else if (strcmp(args[0], "cd")== 0){
       if (args[1] == NULL){
            fprintf(stderr, "Expected argument to 'cd'\n");
       }
       else{
        if(chdir(args[1]) != 0){
            perror("change directory failed");
        }
       }
    return 0;
    }
    return 0;
}

int main () { 
    char *args[COMMAND_SIZE / 2 + 1]; // Allocate space for arguments
    char command[COMMAND_SIZE];
    while (1){
        displayPrompt(); 
        getInput(command);
        parseInput(command, args);
        
        if(execute(args)==1){
            break;
        }       
    }
}