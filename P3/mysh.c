#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdarg.h>
#include <pthread.h>
#include <errno.h>
#include <sys/wait.h>

#include "mysh.h"

#define BUFSIZE 16
#define _POSIX_C_SOURCE 200809L

#define DEBUG 0

typedef struct {
    array_t* arguments;
    char *execpath;
    char *inputfile;
    char *outputfile;
} command;

void cd(char *path){
    if (chdir(path) == -1) {
        fprintf(stderr, "No such file or directory");
    }
}

void pwd(){
    int PATH_SIZE = 4096;
    char mypathname[PATH_SIZE];
    getcwd(mypathname, PATH_SIZE);
    printf("%s\n", mypathname);
    fflush(stdout);
}

int is_built_in(char* str){
    return strcmp(str, "cd") == 0 || strcmp(str, "exit") == 0 || strcmp(str, "pwd") == 0 || strcmp(str, "which") == 0;
}

char* id_command(char* name){
        if(is_built_in(name)){
            return name;
        }

        for(int i = 0; i < strlen(name); i++){
            if(name[i] == '/') return name;
        }
        char* testing[] = {"/usr/local/bin", "/usr/bin", "/bin"};
        for(int i = 0; i < 3; i++){
            size_t size = sizeof(char)*(strlen(testing[i]) + strlen(name) + 2);
            char* temp = (char*)malloc(size);
            temp[0] = '\0';
            strcat(temp, testing[i]);
            strcat(temp, "/");
            strcat(temp, name);
            if(!access(temp, F_OK))
                return temp;
            free(temp);
        }

    return NULL;
}

void which(char *filename){
    if(strcmp(filename,"cd") == 0 || strcmp(filename, "pwd") == 0 || strcmp(filename, "which") == 0 || strcmp(filename, "exit") == 0){
        // error because cmd name is same as built-in
        perror("given a built in command");
        return;
    }
    int PATH_SIZE = 4096;
    char* filepath = id_command(filename);
    if(filepath == NULL){
        perror("invalid command given");
        // error because the file is not found
        return;
    }
    printf("%s", filepath);
}

void printCmd(command* cmd){
    if(cmd->execpath != NULL){
        printf("execution path: %s\n", cmd->execpath);
    } else {
        printf("execution path isn't registered\n");        
    }
    if(cmd->arguments != NULL){
    if(cmd->arguments->length > 0){
        for(int i = 0; i < cmd->arguments->length; i++){
            printf("arg %d: %s ", (i+1), cmd->arguments->data[i]);
        }
        printf("\n");
    } else {
        printf("no arguments given\n");
    }
    }
    if(cmd->inputfile != NULL){
        printf("input file: %s\n", cmd->inputfile);
    } else {
        printf("No input file given\n");
    }
    if(cmd->outputfile != NULL){
        printf("output file: %s\n", cmd->outputfile);
    } else {
        printf("No output file given\n");
    }
}

void cmd_init(command* cmd, unsigned size){
    // cmd = (command*)malloc(sizeof(command));
    cmd->execpath = NULL;
    cmd->inputfile = NULL;
    cmd->outputfile = NULL;
    cmd->arguments = (array_t*)malloc(sizeof(array_t));
    if(size > 0) al_init(cmd->arguments, size);
}
void cmd_free(command* cmd){
    if(cmd->execpath != NULL) free(cmd->execpath);
    if(cmd->inputfile != NULL) free(cmd->inputfile);
    if(cmd->outputfile != NULL) free(cmd->outputfile);
    al_destroy(cmd->arguments);
    free(cmd->arguments);
    free(cmd);
}

enum tok_t { WORD, LT, GT, BAR, NL, EOS };

enum tok_t next_token(char **word){
    char* tempWord = *word;
    if(*tempWord == EOF) return EOS;
    for(int i = 0; tempWord[i] != '\0'; i++){
        switch(tempWord[i]){
            case '|':
                return BAR;
            break;
            case '<':
                return LT;
            break;
            case '>':
                return GT;
            break;
            case '\n':
                return NL;
            break;
            default:
                continue;
        }
    }
    return WORD;
}

void al_init(array_t *L, unsigned cap) {
    L->length = 0;
    L->capacity = cap;
    L->data = malloc(sizeof(char*) * cap);
}
void sb_init(stringBuilder *S, unsigned cap){
    S->length = 0;
    S->capacity = cap;
    S->data = malloc(sizeof(char)*cap);
}

void al_destroy(array_t *L)
{
    for(int i = 0; i < L->length; i++){
        free(L->data[i]);
    }
    free(L->data);
}
void sb_destroy(stringBuilder *S){
    free(S->data);
}

void al_append(array_t *L, char* item, unsigned word_length)
{
    if (L->length == L->capacity) {
        L->capacity *= 2;
        // printf("doubling capacity to %d\n", L->capacity);
        L->data = realloc(L->data, L->capacity * sizeof(char*));
    }

    L->data[L->length] = (char*)malloc((word_length + 1)*sizeof(char));
    strcpy(L->data[L->length], item);
    L->length++;
}

void al_add_sb(array_t *L, stringBuilder* item){
    item->data[item->length] = '\0';
    item->data = realloc(item->data, sizeof(char)*(item->length + 1));
    al_append(L, item->data, item->length + 1);
}

void sb_append(stringBuilder *S, char item){
    if (S->length == S->capacity) {
        S->capacity *= 2;
        // printf("doubling capacity to %d\n", L->capacity);
        S->data = realloc(S->data, S->capacity * sizeof(char*) + 1);
    }
    S->data[S->length] = item;
    S->length++;
}

int built_ins(command** cmd){
    if(cmd[0]->inputfile != NULL){
        perror("can't use intput redirection w/ built-in functions");
        return 1;
    }
    if(cmd[0]->outputfile != NULL){
        perror("can't use output redirection w/ built-in functions");
        return 1;
    }
    if(cmd[1]->execpath != NULL){
        perror("can't use pipes w/ built-in functions");
        return 1;
    }
    int length = cmd[0]->arguments->length;
    switch(cmd[0]->execpath[0]){
        case 'e':
            printf("mysh: exiting\n");
            if(length != 0){
                for(int i = 0; i < cmd[0]->arguments->length; i++)
                    printf("%s ", cmd[0]->arguments->data[i]);
                printf("\n");
            }
            return -1;
        break;
        case 'w':
            if(length == 1)
                which(cmd[0]->arguments->data[0]);
            else if(length == 0){
                perror("did not provide which any commands");
                return 1;
            } else {
                perror("gave which too many arguments");
                return 1;
            }
        break;
        case 'p':
            if(length == 0)
                pwd();
            else{
                perror("gave pwd arguments");
                return 1;
            }
        break;
        case 'c':
            if(length == 1)
                cd(cmd[0]->arguments->data[0]);
            else if(length == 0){
                perror("did not provide cd any commands");
                return 1;
            } else {
                perror("gave cd too many arguments");
                return 1;
            }
        break;
    }
    return 0;
}

int trigger(command* cmd){

}

array_t* prep_args(command* cmd){
    array_t *trueArgList = (array_t*) malloc(sizeof(array_t));
        int l = cmd->arguments->length+2;
        if(DEBUG) printf("%d\n", l);
        al_init(trueArgList, l);
        for(int i = 0; i < l; i++){
            if(i == 0){
                al_append(trueArgList, cmd->execpath, strlen(cmd->execpath));
            } else if(i == l - 1) {
                trueArgList->data[i] = NULL;
            } else {
                al_append(trueArgList, cmd->arguments->data[i-1],strlen(cmd->arguments->data[i-1]));
            }
        }
    return trueArgList;
}

int execCommand(command** cmd){
    if(cmd[0]->execpath != NULL){
        if(DEBUG) printf("%d\n", is_built_in(cmd[0]->execpath));
        if(cmd[1]->execpath == NULL && !is_built_in(cmd[0]->execpath)){
                        

            array_t* trueArgList = prep_args(cmd[0]);
            pid_t child = fork();

            if(child < 0){
                perror("fork failed");
                return 2;
            }

            if(child == 0){
                int inFlag = 0, outFlag = 0;
                int inFile = 0, outFile = 0;
                if(cmd[0]->inputfile != NULL){
                    inFlag = 1;
                    inFile = open(cmd[0]->inputfile, O_RDONLY, 0644);
                    if(inFile < 0){
                        perror("openning the file to read in failed");
                        return 2;
                    }
                    if(dup2(inFile, STDIN_FILENO) < 0){
                        perror("dup2 failed input file");
                        return 2;
                    }
                    close(inFile);

                } 
                if(cmd[0]->outputfile != NULL){
                    outFlag = 1;
                    if(access(cmd[0]->outputfile, F_OK)){
                        outFile = open(cmd[0]->outputfile, O_WRONLY|O_TRUNC|O_CREAT, 0640);
                        if(outFile < 0){
                            fprintf(stderr, "unable to create the file: %s", cmd[0]->outputfile);
                        }
                    } else {
                        outFile = open(cmd[0]->outputfile,O_WRONLY|O_TRUNC, 0640);
                        if (outFile == -1) {
                            fprintf(stderr, "unable to write to the file: %s", cmd[0]->outputfile);
                            return 2;
                        }   
                    }
                    if(dup2(outFile, STDOUT_FILENO) < 0){
                        perror("dup2 failed output file");
                        return 2;
                    }
                    close(outFile);
                }
                execv(cmd[0]->execpath, trueArgList->data);

                perror("execv");
                exit(EXIT_FAILURE);
                // if(inFlag){
                //     close(inFile);
                // }
                // if(outFile){
                //     close(outFile);
                // }

            }
            int status;

            child = wait(&status);
            if(WIFEXITED(status)){
                // printf("command %s exited normally", cmd[0]->execpath);
            } else if(WIFSIGNALED(status)){
                printf("command %s interrupted via signal", cmd[0]->execpath);
            } else {
                printf("command %s exited with status: %d", cmd[0]->execpath, WEXITSTATUS(status));
            }

            
            al_destroy(trueArgList);
            free(trueArgList);

        } else if(is_built_in(cmd[0]->execpath)){
            return built_ins(cmd);
        } else if(cmd[1]->execpath != NULL){
            if(!is_built_in(cmd[1]->execpath)){
                array_t* trueArgList0 = prep_args(cmd[0]);
                array_t* trueArgList1 = prep_args(cmd[1]);

                int pipefd[2];
                if(pipe(pipefd) < 0){
                    perror("pipe failed");
                    return 2;
                }

                pid_t pid1 = fork();
                if (pid1 < 0) {
                    perror("fork failed for first command");
                    return 2;
                }

                if(pid1 == 0){
                    close(pipefd[0]);
                    int inFile = 0;
                    if(cmd[0]->inputfile != NULL){
                        inFile = open(cmd[0]->inputfile, O_RDONLY, 0644);
                        if(inFile < 0){
                            perror("openning the file to read in failed");
                            return 2;
                        }
                        if(dup2(inFile, STDIN_FILENO) < 0){
                            perror("dup2 failed input file");
                            return 2;
                        }
                        close(inFile);
                    }
                    if(cmd[0]->outputfile != NULL){
                        perror("cannot redirect the output of the first command in a pipe");
                        return 2;
                    }
                    if(dup2(pipefd[1], STDOUT_FILENO) < 0){
                        perror("dup2 pipe firs tcommand failed");
                        return 2;
                    }
                    close(pipefd[1]);
                    execv(cmd[0]->execpath, trueArgList0->data);
                    perror("execv");
                    exit(EXIT_FAILURE);   
                }

                pid_t pid2 = fork();
                if (pid2 < 0) {
                    perror("fork failed for first command");
                    return 2;
                }

                if(pid2 == 0){
                    int outFile = 0;
                    close(pipefd[1]);
                    if(cmd[1]->outputfile != NULL){
                        if(access(cmd[1]->outputfile, F_OK)){
                            outFile = open(cmd[1]->outputfile, O_WRONLY|O_TRUNC|O_CREAT, 0640);
                            if(outFile < 0){
                                fprintf(stderr, "unable to create the file: %s", cmd[0]->outputfile);
                            }
                        } else {
                            outFile = open(cmd[1]->outputfile,O_WRONLY|O_TRUNC, 0640);
                            if (outFile == -1) {
                                fprintf(stderr, "unable to write to the file: %s", cmd[0]->outputfile);
                                return 2;
                            }   
                        }
                        if(dup2(outFile, STDOUT_FILENO) < 0){
                            perror("dup2 failed output file");
                            return 2;
                        }
                        close(outFile);
                    }

                    if(cmd[1]->inputfile != NULL){
                        perror("cannot redirect the input of the second command in a pipe");
                        return 2;
                    }
                    if(dup2(pipefd[0], STDIN_FILENO) < 0){
                        perror("dup2 pipe second command failed");
                        return 2;
                    }
                    close(pipefd[0]);
                    execv(cmd[1]->execpath, trueArgList1->data);
                    perror("execv");
                    exit(EXIT_FAILURE);   
                }
                close(pipefd[0]);
                close(pipefd[1]);
                int status1;
                int status2;
                waitpid(pid1, &status1, 0);
                if(WIFEXITED(status1)){
                // printf("command %s exited normally", cmd[0]->execpath);
                } else if(WIFSIGNALED(status1)){
                    printf("command %s interrupted via signal", cmd[0]->execpath);
                } else {
                    printf("command %s exited with status: %d", cmd[0]->execpath, WEXITSTATUS(status1));
                }
                waitpid(pid2, &status2, 0);                
                if(WIFEXITED(status2)){
                // printf("command %s exited normally", cmd[0]->execpath);
                } else if(WIFSIGNALED(status2)){
                    printf("command %s interrupted via signal", cmd[1]->execpath);
                } else {
                    printf("command %s exited with status: %d", cmd[1]->execpath, WEXITSTATUS(status2));
                }
                
            } else {
                perror("can't use pipes with built in functions");
                return 2;
            }
        }
    }
    return 0;
}


int isValid(char* canidate, char* search){
    int n = strlen(canidate);
    int m = strlen(search);
    int i = 0;
    int j = 0;
    int match = 0;
    int startIndex = -1;
    while(i < n){
        if(j < m && canidate[i] == search[j]){
            i++;
            j++;
        } else if(j < m && search[j] == '*'){
            startIndex = j;
            match = i;
            j++;
        } else if(startIndex != -1){
            j = startIndex+ 1;
            match++;
            i = match;
        } else {
            return 0;
        }
    }
    while(j < m && search[j] == '*'){
        j+= 1;
    }
    return j == m;
}

void wildAdd(array_t* args, char* search){
    int starloc = 0, slashloc = 0, starCount = 0, slashCount = 0 ;
    
    int n = strlen(search);
    for(int i = 0; i < n; i++){
        if(search[i] == '*'){
            starloc = i;
            starCount++;
        }
        if(search[i] == '/'){
            slashloc = i;
            slashCount++;
        }
    }

    if(starCount == 0){
        al_append(args, search, n);
    } else {
        if(starCount > 1){
            perror("too many wildcards");
            return;
        } 
        if(slashloc > starloc){
            perror("wildcard is in path, not filename");
            return;
        }
        char dirname[4096];
        if(slashCount == 0){
            getcwd(dirname, 4096);
        } else {
            strncpy(dirname, search, (size_t) slashloc);
        }
        DIR *dir = opendir(dirname);
        if(!dir){
            fprintf(stderr, "incorrect path given: %s", dirname);
            // perror();
            return;
        }
        char* trueSearch;
        if(slashCount > 0) trueSearch = search + slashloc + 1;
        else trueSearch = search;

        struct dirent *ent;
        struct stat buf_st;
        char pname[4096];
        int hasMatched = 0;
        while((ent = readdir(dir))){
            char *dname = ent->d_name;
            if(dname[0] == '.') continue;

            strcpy(pname, dirname);
            strcat(pname, "/");
            strcat(pname, dname);

            if(!stat(pname, &buf_st)){
                if(S_ISREG(buf_st.st_mode)){
                    if(isValid(dname, trueSearch)){
                        if(!hasMatched) hasMatched = 1;
                        al_append(args, pname, strlen(pname));
                    }
            }
        }       
        
    }
    if(!hasMatched)
            al_append(args, search, n);
    closedir(dir);
    }

}




command** rawToCommand(array_t* words){
    command** cmd = (command **)malloc(sizeof(command*) * 2);
    if (!cmd) {
        perror("malloc failed for cmd");
        return NULL;
    }

    cmd[0] = (command *)malloc(sizeof(command));
    cmd[1] = (command *)malloc(sizeof(command));
    cmd_init(cmd[0], 10);
    cmd_init(cmd[1], 10);    
    int first = 1;
    int ind = 0;
    for(int i = 0; i < words->length; i++){
        
        if(DEBUG) printf("%d\n", next_token((words->data)+i));
        
        switch(next_token((words->data)+i)){
            case WORD:
                if(first){
                    char* fullpath = id_command(words->data[i]);
                    if(fullpath == NULL){
                        printf("fullpath is NULL\n");
                        printf("");
                        // exit(EXIT_FAILURE);
                        return cmd;
                    } else {
                    cmd[ind]->execpath = (char*)malloc(sizeof(char)* (strlen(fullpath) + 1));
                    strcpy(cmd[ind]->execpath, fullpath);
                    first = 0;
                    if(strcmp(fullpath, words->data[i]) != 0)
                        free(fullpath);
                    }
                    
                } else {
                    wildAdd(cmd[ind]->arguments, words->data[i]);
                    // al_append(cmd[ind]->arguments, words->data[i], strlen(words->data[i]));
                }
                break;
            case LT:
                if(i+1 < words->length){
                    if(next_token((words->data)+i+1) == WORD){
                        int length = strlen(words->data[i+1]);
                        cmd[ind]->inputfile = (char*)malloc(sizeof(char)*(length+1));
                        strcpy(cmd[ind]->inputfile,words->data[i+1]);
                        i++;
                    } 
                }
                else {

                }
                break;
            case GT:
                if(i+1 < words->length){
                    if(next_token((words->data)+i+1) == WORD) {
                        int length = strlen(words->data[i+1]);
                        cmd[ind]->outputfile = (char*)malloc(sizeof(char)*(length+1));
                        strcpy(cmd[ind]->outputfile,words->data[i+1]);
                        i++;
                    }
                }
                else {

                }

                break;
            case BAR:
                if(i+1 < words->length && !first){
                    if(next_token((words->data)+i+1) == WORD){
                        if(ind == 0){
                        first = 1;
                        ind++;
                        // al_init(cmd[ind]->arguments, 10);
                        // cmd[1] = rawToCommand((words->data)+i+1)[0];
                        } else {
                            perror("more than one pipe included\n");
                            free(cmd[0]->execpath);
                            cmd[0]->execpath = NULL;
                            return cmd;
                        }
                    } else {
                        if(cmd[0]->execpath != NULL){
                            free(cmd[0]->execpath);
                            cmd[0]->execpath = NULL;
                        }
                        perror("mallformed pipe\n");
                        return cmd;  
                    }
                }
                else{
                    if(cmd[0]->execpath != NULL){
                            free(cmd[0]->execpath);
                        }
                    perror("mallformed pipe\n");
                    return cmd;
                } 

                break;
            case EOS:
            case NL:
                if(!first)
                    return cmd;
                else
                    return NULL;
                break;
            default:
                printf("Unexpected token: %d\n", next_token((words->data) + i));
                break;
        }
    }
    return cmd;
}






void terminal(int fd){
    int is_batch;
    if(!DEBUG)
        is_batch = 0;
    is_batch = !isatty(fd);


    int bytes = 0;
    array_t* parsed = (array_t*) malloc(sizeof(array_t));
    al_init(parsed, 10);

    int word_len_max = BUFSIZE;
	// char *word = (char *)malloc(sizeof(char) * (word_len_max + 1));
    int in_word = 0;
    char buf[BUFSIZE] = {0};

    stringBuilder* word = (stringBuilder*) malloc(sizeof(stringBuilder));
    sb_init(word, word_len_max+1);


    if(!is_batch) {
        if(DEBUG) {printf("Enum Values: WORD=%d, LT=%d, GT=%d, BAR=%d, NL=%d, EOS=%d\n", WORD, LT, GT, BAR, NL, EOS);}
        printf("Welcome to my shell!\nmysh> ");
        fflush(stdout);
    }
    while((bytes = read(fd, buf, BUFSIZE)) > 0){
        for(int i = 0; i < bytes; i++){
            char str[2] = "\0";
            switch(buf[i]){
                case '|':
                case '<':
                case '>':
                    if(in_word){
                        in_word = 0;
                        al_add_sb(parsed, word);
                        sb_destroy(word);
                        word_len_max = BUFSIZE;
                        sb_init(word, word_len_max + 1);
                    }
                    str[0] = buf[i];
                    al_append(parsed, str, 2);
                
                break;

                case '\n':
                case EOF:
                    if(in_word){
                        in_word = 0;
                        al_add_sb(parsed, word);
                        word_len_max = BUFSIZE;
                        sb_destroy(word);
                        sb_init(word, word_len_max + 1);
                    } 
                    if(buf[i] == '\n'){
                        str[0] = buf[i];
                        al_append(parsed, str, 2);
                    }
                   
                   


                    if(DEBUG){
                        for(int i = 0; i < parsed->length; i++){
                            if(parsed->data[i][0] != '\n') printf("%d: %s ", (i+1), parsed->data[i]);
                        }
                    }
                    command** cmd;
                    cmd = rawToCommand(parsed);

                    if (cmd == NULL) {
                       printf("cmd is NULL, cannot proceed.\n");
                       fflush(stdout);
                        return;
                    } 
                    if(DEBUG){
                        printCmd(cmd[0]);
                        printCmd(cmd[1]);
                    }

                    int returnStatus = execCommand(cmd);
                    
                    al_destroy(parsed);

                    cmd_free(cmd[0]);
                    cmd_free(cmd[1]);
                    free(cmd);
                    if(returnStatus == -1){
                        sb_destroy(word);
                        free(word);
                        free(parsed);
                        exit(EXIT_SUCCESS);
                    }

                    if(buf[i] == '\n'){
                        parsed = (array_t*)malloc(sizeof(array_t));
                        al_init(parsed, 10);
                    } else {
                        sb_destroy(word);
                        free(word);
                        free(parsed);
                        exit(EXIT_SUCCESS);
                    }
                    if(!is_batch){
                        printf("\nmysh> ");
                        fflush(stdout);                        
                    }

                break;                    

                case ' ':
                case '\t':
                    if(in_word){
                        in_word = 0;
                        al_add_sb(parsed, word);
                        sb_destroy(word);
                        word_len_max = BUFSIZE;
                        sb_init(word, word_len_max + 1);
                    } 
                break;

                default:
                    if(!in_word) in_word = 1;
                    sb_append(word, buf[i]);
                break;
            }
        }
        // if(!is_batch) printf("\nmysh> ");
    }

    sb_destroy(word);
    free(word);
    al_destroy(parsed);
    free(parsed);
}


int main(int argc, char** argv){
    int fileInput = 0;
    if(argc > 2){
        fprintf(stderr, "%d is too many arguements, can only recieve a max of 1\n", argc - 1);
        exit(2);
    } else if(argc == 2){
        char* filename = argv[1];
        struct stat buf_st;
        if(!stat(filename, &buf_st)){
            if(S_ISREG(buf_st.st_mode)){
                fileInput = open(filename, O_RDONLY);
                terminal(fileInput);
                close(fileInput);
            } else {
                fprintf(stderr, "did not provide a valid file");
                exit(2);
            }
        }
    } else {
        terminal(fileInput);
    }
}