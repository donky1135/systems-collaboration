#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

int stdout_origin;
int stdin_origin;

void set_fd_origins(){ // hold standard input/output so that can return to it. must call before any dup
    stdout_origin = dup(stdout);
    stdin_origin = dup(stdin);
}

void cd(char *path){
    if (chdir(path) == -1) {
        fprintf(stdout, "No such file or directory");
    }
}

void pwd(){
    int PATH_SIZE = 4096;
    char pathname[PATH_SIZE];
    getcwd(pathname, PATH_SIZE);
    fprintf(stdout, "%s", pathname);
}

void which(char *filename){
    if(strcmp(filename,"cd") == 0 || strcmp(filename, "pwd") == 0 || strcmp(filename, "which") == 0 || strcmp(filename, "exit") == 0){
        // error because cmd name is same as built-in
        return 1;
    }
    int PATH_SIZE = 4096;
    char filepath[PATH_SIZE];
    // filepath = find_program(filename);
    if(filepath == NULL){
        // error because the file is not found
        return 1;
    }
    fprintf(stdout, "%s", filepath);
}

void rd_in(char *filename){ // redirection for input
    int fd = open(filename, O_RDONLY);
    close(stdin);
    dup2(fd, stdin);
    close(fd);
}

void rd_out(char *filename){ // redirection for output
    int fd = open(filename, O_WRONLY);
    close(stdout);
    dup2(fd, stdout);
    close(fd);
}

int *my_pipe(){
    /*
        new pipe for cmd1 | cmd2
        call this before fork() for first command
        call l_pipe() after fork() for cmd1
        r_pipe() after fork() for cmd2
    */
    int *p = (int *)(malloc(sizeof(int) * 2));
    pipe(p);
    return p;
}

void l_pipe(int *p){ // redirection the output of the cmd leftside of | into pipe
    dup2(p[1], stdout);
    close(p[0]);
    close(p[1]);
    free(p);
}

void r_pipe(int *p){ // redirection the input of the cmd rightside of | into pipe
    dup2(p[0], stdin);
    close(p[0]);
    close(p[1]);
    free(p);
}

void reset_stdout(){ // make stdout refers standard output, useful for stepping new line
    close(stdout);
    dup2(stdout_origin, stdout);
}

void reset_stdin(){ // make stdin refers standard input, useful for stepping new line
    close(stdin);
    dup2(stdin_origin, stdin);
}