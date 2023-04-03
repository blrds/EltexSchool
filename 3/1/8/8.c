#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>

void parent(int pipe){
    printf("parent start\n");
    FILE *fd=fdopen(pipe, "r");
    FILE *f=fopen("a.txt", "w");
    int c;
    printf("parent read\n");
    while((c=fgetc(fd))!=EOF){
        printf("parent read\n");
        fwrite(&c,sizeof(char),1,f);
        printf("%c", (char)c);
    }
    printf("parent read end\n");
    fclose(f);
    fclose(fd);
    printf("parent end\n");
}

void child(int count, int pipe){
    srand(time(NULL));
    FILE *fd=fdopen(pipe, "w");
    int j=0;
    for(int i=0;i<count;i++){
        j=rand()%20;
        fprintf(fd, "%d ", j);
    }
    fclose(fd);
}

int main(int argc, char* argv[]){
    if(argc!=2)exit(EXIT_FAILURE);
    __pid_t pid;
    int _pipe[2];
    pipe(_pipe);
    pid=fork();
    if(pid==0)child(atoi(argv[1]), _pipe[1]);
    else parent(_pipe[0]);
    exit(EXIT_SUCCESS);
}