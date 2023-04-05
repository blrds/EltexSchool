#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>

void parent(int pipe){
    printf("parent start\n");
    FILE *fd=fdopen(pipe, "r");
    int c;
    char str[256]={0};
    printf("parent read\n");
    int i=0;//int из-за fgetc(EOF=-1)
    while((c=fgetc(fd))!=EOF){
        if(c==EOF)break;
        printf("parent read\n");
        str[i]=c;
        i++;
    }
    str[i]='\0';
    printf("parent read end\n");
    fclose(fd);
    printf("%s\n", str);
    FILE *f=fopen("a.txt", "w");
    fwrite(&str,sizeof(str),1,f);
    fclose(f);
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