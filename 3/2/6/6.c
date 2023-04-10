#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>
#include<string.h>
#include<signal.h>

int canChildReadTheFile=1;

void parent(int pipe, __pid_t child){
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
    kill(child,SIGUSR1);
    FILE *f=fopen("a.txt", "w");
    sleep(5);//for test purpose
    fwrite(&str,sizeof(char),strlen(str),f);
    fclose(f);
    kill(child,SIGUSR2);
}

void USR1Listener(int signal){
    if(signal!=SIGUSR1)return;
    canChildReadTheFile=0;
}

void USR2Listener(int signal){
    if(signal!=SIGUSR2)return;
    canChildReadTheFile=1;
}

void child(int count, int pipe){
    signal(SIGUSR1,USR1Listener);
    signal(SIGUSR2,USR2Listener);
    srand(time(NULL));
    FILE *fd=fdopen(pipe, "w");
    int j=0;
    for(int i=0;i<count;i++){
        j=rand()%20;
        fprintf(fd, "%d ", j);
    }
    fprintf(fd,"%c", '\0');
    fclose(fd);
    while(!canChildReadTheFile){
        printf("Child is waiting\n");
        sleep(1);
    }
    fd=fopen("a.txt", "r");
    char str[256]={0};
    fgets(str,256,fd);
    fclose(fd);
    printf("Child red: %s\n",str);
}

int main(int argc, char* argv[]){
    if(argc!=2)exit(EXIT_FAILURE);
    __pid_t pid;
    int _pipe[2];
    pipe(_pipe);
    pid=fork();
    if(pid==0)child(atoi(argv[1]), _pipe[1]);
    else {
        parent(_pipe[0], pid);
        wait(NULL);    
    }
    exit(EXIT_SUCCESS);
}