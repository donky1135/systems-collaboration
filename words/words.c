#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "words.h"

#ifndef DEBUG
#define DEBUG 1
#endif

void al_init(array_t *L, unsigned cap) {
    L->length = 0;
    L->capacity = cap;
    L->key = malloc(sizeof(char*)*cap);
    L->value = malloc(sizeof(int) * cap);
}

void al_destroy(array_t *L) {
    free(L->key);
    free(L->value);
}
void al_add(array_t *L, char* string){
    if (L->length == L->capacity) {
        L->capacity *= 2;
        if(DEBUG) printf("doubling capacity to %d\n", L->capacity);
        L->key = realloc(L->key, L->capacity * sizeof(char*));
        L->value = realloc(L->value, L->capacity * sizeof(int));
    }
    
    int found = 0;
    for(int i = 0; i < L->length; i++){
        if(!strcmp(string, L->key[i])){
            L->value[i]++;
            found = 1;
            break;
        }
    }

    if(!found){
        L->key[L->length] = (char*)malloc(sizeof(char)*(strlen(string)+1));
        strcpy(L->key[L->length], string);
        L->value[L->length] = 1;
        L->length++;    
    }
}

void al_print(array_t *L){

}

void traverse(char *path)
{
    int pathlen = strlen(path);
    char *fpath;
    DIR *dp = opendir(path);
    if (!dp) {
        perror(path);
        return;
    }
    struct dirent *de;
    while ((de = readdir(dp)) != NULL) {
        if (de->d_name[0] == '.') continue;
        if(DEBUG) printf("%s\n", de->d_name);
        
        int namelen = strlen(de->d_name);
        fpath = (char *) malloc(pathlen + 1 + namelen + 1);
        memcpy(fpath, path, pathlen);
        fpath[pathlen] = '/';
        memcpy(fpath + pathlen + 1, de->d_name, namelen + 1);
        
        if(DEBUG) printf("%s\n", fpath);


        struct stat sb;
        stat(fpath, &sb);
        if (S_ISDIR(sb.st_mode)) {
            traverse(fpath);
        } else if(namelen > 4){
            if(de->d_name[namelen -4] == '.' && de->d_name[namelen -3] == 't' && de->d_name[namelen -2] == 'x' && de->d_name[namelen -1] == 't'){
                if(DEBUG) printf("valid file\n");
            }
        }
        free(fpath);
    }
    closedir(dp);
}

int main(int argc, char** argv){
    printf("Hello world!\n");
    if(argc > 1) traverse(argv[1]);
}