#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>
#include<string.h>

void parent(int pipe){
    FILE *fd=fdopen(pipe, "r");
    int c;
    char str[256]={0};
    int i=0;//int из-за fgetc(EOF=-1)
    while(1){
        c=fgetc(fd);//будет ждать нового символа
        str[i]=(char)c;
        i++;
        if(c=='\0')break;//спец символ, чтобы понять что пайп все
    }
    fclose(fd);
    printf("%s\n", str);
    FILE *f=fopen("a.txt", "w");
    fwrite(&str,sizeof(char),strlen(str),f);
    fclose(f);
}

void child(int count, int pipe){
    srand(time(NULL));
    FILE *fd=fdopen(pipe, "w");
    int j=0;
    for(int i=0;i<count;i++){
        j=rand()%20;
        fprintf(fd, "%d ", j);
    }
    fprintf(fd,"%c", '\0');
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