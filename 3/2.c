#include<stdio.h>
#include<unistd.h>
#include<errno.h>
#include<stdlib.h>

void goingToDie(){
    printf("%d is going to die\n", getpid());
}

int main(int argc, char *argv[]){
    atexit(goingToDie);
    fork();
    for(int i=1;i<argc;i++)
        printf("%s\n", argv[i]);
    
    exit(EXIT_SUCCESS);
}