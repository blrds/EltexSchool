#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<time.h>

void main(){
    int codes[]={SIGINT,SIGQUIT, SIGABRT, SIGKILL, SIGTERM,SIGTSTP, SIGSTOP};
    for(int i=0;i<(sizeof(codes)/sizeof(codes[0]));i++){
        __pid_t pid=fork();
        if(pid==0){
            time_t rtime;
            struct tm * timeinfo;
            char path[256]={0};
            sprintf(path,"%d.txt",i);
            FILE* f=fopen(path,"w+");
            fclose(f);
            printf("child %d: path=%s\n",i,path);
            int j=0;
            while(1){
                f=fopen(path,"a");
                fprintf(f,"%d\n",j++);
                fclose(f);
                time(&rtime);
                timeinfo=localtime(&rtime);
                printf("child %d:%d at %s\n",i,j, asctime(timeinfo));
                sleep(1);
            }
        }
        else{
            sleep(10);
            printf("parent %d: sending %d\n", i, codes[i]);
            kill(pid,codes[i]);
            if(codes[i]==SIGSTOP || codes[i]==SIGTSTP){
                sleep(5);
                kill(pid,SIGCONT);
                sleep(5);
                kill(pid,SIGQUIT);
            }
            printf("parent %d: signal sent\n", i);
            wait(NULL);
        }
    }
}