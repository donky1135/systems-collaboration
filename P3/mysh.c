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

#include "mysh.h"

#define BUFSIZE 16
#define _POSIX_C_SOURCE 200809L

#define DEBUG 0

typedef struct {
    array_t arguments;
    char *execpath;
    char *inputfile;
    char *outputfile;
} command;

enum tok_t { WORD, LT, GT, BAR, NL, EOS };

enum tok_t next_token(char **word){
    char* tempWord = *word;
    if(tempWord == EOF) return EOS;
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

    L->data[L->length] = (char*)malloc(word_length*sizeof(char));
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

int execCommand(command** cmd){
    if(cmd[0] != NULL){
        if(cmd[1] == NULL){
 
            if(cmd[0]->inputfile != NULL){
                
            }
            if(cmd[0]->outputfile != NULL){

            }


        } else {

        }
    }
    return 0;
}


char* id_command(char* name){
        for(int i = 0; i < strlen(name); i++){
            if(name[i] == '\\') return name;
        }
        char* testing[] = {"/usr/local/bin", "/usr/bin", "/bin"};
        for(int i = 0; i < 3; i++){
            char temp[] = "";
            strcat(temp, testing[i]);
            strcat(temp, "/");
            strcat(temp, name);
            if(!access(temp, F_OK))
                return temp;
        }
    return NULL;
}


command** rawToCommand(array_t* words){
    command* cmd[2] = {};
    cmd[0] = (command*)malloc(sizeof(command));
    array_t* argList = (array_t*)malloc(sizeof(array_t));
    int first = 1;
    for(int i = 0; i < words->length; i++){
        switch(next_token((words->data)+i)){
            WORD:
                if(first){
                    cmd[0]->execpath = id_command(words->data[i]);
                    first = 0;
                } else {
                    al_append(argList, words->data[i], strlen(words->data[i]));
                }
                break;
            LT:
                if(i+1 < words->length)
                    if(next_token((words->data)+i+1) == WORD) cmd[0]->inputfile = words->data[i+1];
                break;
            GT:
                if(i+1 < words->length)
                    if(next_token((words->data)+i+1) == WORD) cmd[0]->outputfile = words->data[i+1];
                break;
            BAR:
                if(i+1 < words->length)
                    if(next_token((words->data)+i+1) == WORD) cmd[1] = rawToCommand((words->data)+i+1)[0];
                break;
            EOS:
                if(!first)
                    return cmd;
                else
                    return NULL;
                break;
            NL:
                if(!first)
                    return cmd;
                else
                    return NULL;
                break;
        }
    }
    return cmd;
}






void terminal(int fd){
    int is_batch = 1;
    if(!DEBUG)
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
                    if(in_word){
                        in_word = 0;
                        al_add_sb(parsed, word);
                        word_len_max = BUFSIZE;
                        sb_init(word, word_len_max + 1);
                    } 
                    str[0] = buf[i];
                    al_append(parsed, str, 2);
                    
                    command** cmd = rawToCommand(parsed);

                    execCommand(cmd);



                    for(int i = 0; i < parsed->length; i++){
                        if(parsed->data[i][0] != '\n') printf("%d: %s ", (i+1), parsed->data[i]);
                    }


                    al_destroy(parsed);
                    parsed = (array_t*)malloc(sizeof(array_t));
                    al_init(parsed, 10);
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
    free(word);
    al_destroy(parsed);

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