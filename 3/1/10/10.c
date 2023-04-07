#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

int main(int argc, char* argv[]){
    int status;
    char *args[2];
    int _pipe[2];
    pipe(_pipe);
    args[0] = "/bin/ls";        // first arg is the full path to the executable
    args[1] = NULL;             // list of args must be NULL terminated
    
    if ( fork() == 0 ){
        close(_pipe[0]);
        dup2(_pipe[1], 1);
        execv( args[0], args ); // child: call execv with the path and the args
    }else{
        close(_pipe[1]);
        char str[256]={0};
        wait( &status );
        read(_pipe[0],&str, sizeof(str));
        printf("A%sA\n", str);
    }
    
    exit(EXIT_SUCCESS);
}