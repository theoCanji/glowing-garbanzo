#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024
#define COMMAND_SIZE 1024
#define HISTORY_SIZE 100


void handler(int sig) {
    //printf("handler called\n");
    wait(NULL);
}


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
void getInput(char* command){

    if (fgets(command, COMMAND_SIZE, stdin ) == NULL){
        perror("fgets() error");
    }
}
void removeValues(char** args, int index){
    int i = index; 
    while (args[i]!= NULL){
        args[i] = args[i+2];
        i++; 
    }
    args[i] = NULL;
}

// parse the input using strtok
void parseInput(char* command, char **args){

    command[strcspn(command, "\n")] = 0; //remove newline


    char* token = strtok(command, " ");
    int i = 0; 
    while (token != NULL){
        args[i] = token;
        token = strtok(NULL, " "); //turns out this function just picks up where the last call left off
        i++; 
    }
    args[i] = NULL; //last index is set to NULL
    args[i+1] = NULL;

    //meep meep beep beep down the street street
}

//execute the command (exit and cd). non-zero value is returned if the command is not executed successfully
int execute_internal_commands(char** args, char** history, int histCounter){
    if (strcmp(args[0], "exit") == 0 ){
        printf("Goodbye!\n");
        return 2; 
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
        return 1;
    }
    else if (strcmp(args[0], "history") == 0){
        for (int i = 0; i < histCounter; i++){
            printf("%d %s\n", i+1, history[i]);
        }
        return 1;
    }
    //extra credit: 
    else if (args[0][0] == '!' && args[0][1] != '\0'){
        int histIndex = atoi(&args[0][1]) - 1; // Convert !{i} to index in history array
        if(histIndex >= 0 && histIndex < histCounter){
            printf("Executing command from history: %s\n", history[histIndex]);
            strcpy(args[0], history[histIndex]); // Replace current command with history command
            parseInput(history[histIndex], args); // Re-parse command from history
        } else {
            
            printf("No such command in history.\n");
            return 1;
        }
    }
    return 0;
    }


FILE* input_output_redirect(char* command, char* input_output, char* file){
    FILE* fp = NULL;
    if(strcmp(input_output, "<") == 0){ // input
        fp = freopen(file, "r", stdin);
        if (fp== NULL){
            perror("freopen");
        }
    }
    else if(strcmp(input_output, ">") == 0){ // output
        fp = freopen(file, "w", stdout);
        if (fp == NULL) {
            perror("freopen");
        }
    }
    return fp;
}

int is_background(char** args) {
    int i = 0;
    while (args[i+1] != NULL)
    {
        i++;
    }
    
    if(args[i][0] == '&'){
        args[i] = NULL;
        return 1;
    } else {
        return 0;
    }
}

//execute external commands function
void execute_external_command(char** args, int bg){
    //fork the child process
    pid_t pid = fork();
    
    if (pid == 0){ //in child process
        printf("Child execute command %p: ", args[0]);
        
        int i = 0; //index for the args array
        int inout_redir = 0; // 0 is false, 1 is true, operator is used to check if the input/output redirection is present
        FILE* file = NULL;
        
        while(args[i] != NULL && inout_redir != 1){
            if (args[i][0] == '<' || args[i][0] == '>'){
                file = input_output_redirect(args[0], args[i], args[i+1]);
                inout_redir = 1; //condition to break the loop
            }
            
            i++;
        }
        // remove args[i-1] and args[i] from args
        if(inout_redir == 1) {
            removeValues(args, i-1);
        }
        
        
        if (execvp(args[0], args) == -1){ //have the child process perform an execvp
            perror("execvp failed: "); 
            exit(EXIT_FAILURE);  // Exit child process if execvp fails
        }
        if(inout_redir == 1) {
            fclose(file);
        }
        
    } else if (pid > 0) { // in parent process
        signal(SIGCHLD, handler);
        int status;
        if(bg == 0) {
            waitpid(pid, &status, 0); // have the parent process wait for the child to finish.
        }
    
    } else {
        perror("fork failed");
    }
}


int main () { 
    char *args[COMMAND_SIZE / 2 + 1]; // Allocate space for arguments
    char command[COMMAND_SIZE];
    int int_cmd = 0; // internal command return
    
    //history update
    char *history[HISTORY_SIZE]; // history command list
    int histCounter = 0;
    int eqHist;

    while (int_cmd < 2){
        displayPrompt(); 
        getInput(command);
        parseInput(command, args);

        eqHist = strcmp(command, "history");
        printf("strcmp: %d\n", eqHist);
        if (histCounter < HISTORY_SIZE && command[0] != '!' && eqHist!= 0 && strcmp(command, "history") != 0){
            history[histCounter] = strdup(command);
            histCounter++;
        }

        int_cmd = execute_internal_commands(args, history, histCounter);
        if(int_cmd == 0){
            execute_external_command(args, is_background(args));
        }
    }
}