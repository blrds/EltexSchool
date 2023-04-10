#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<time.h>

void main(){
        __pid_t pid=fork();
        if(pid==0){
            signal(SIGINT, SIG_IGN);
            time_t rtime;
            struct tm * timeinfo;
            char path[256]={0};
            sprintf(path,"2.txt");
            FILE* f=fopen(path,"w+");
            fclose(f);
            printf("child: path=%s\n",path);
            int j=0;
            while(1){
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
            for(int i=0;i<5;i++){
                sleep(3);
                printf("parent: sending SIGINT\n");
                kill(pid,SIGINT);
                printf("parent: signal sent\n");
            }
            printf("parent : sending SIGQUIT\n");
            kill(pid,SIGQUIT);
            printf("parent: signal sent\n");
            wait(NULL);
        }
    
}