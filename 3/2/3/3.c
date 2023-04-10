#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<time.h>
int counter=3;

void SIGINTListener(int signal){
    if(signal!=SIGINT)return;
    else counter--;
}

void main(){
        __pid_t pid=fork();
        if(pid==0){
            signal(SIGINT, SIGINTListener);
            time_t rtime;
            struct tm * timeinfo;
            char path[256]={0};
            sprintf(path,"out.txt");
            FILE* f=fopen(path,"w+");
            fclose(f);
            printf("child: path=%s\n",path);
            int j=0;
            while(counter){
                f=fopen(path,"a");
                fprintf(f,"%d\n",j++);
                fclose(f);
                time(&rtime);
                timeinfo=localtime(&rtime);
                printf("child:%d at %s\n",j, asctime(timeinfo));
                sleep(1);
            }
        }
        else{
            for(int i=0;i<4;i++){
                sleep(3);
                printf("parent: sending SIGINT\n");
                kill(pid,SIGINT);
                printf("parent: signal sent\n");
            }
            wait(NULL);
        }
    
}