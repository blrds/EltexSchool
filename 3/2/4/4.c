#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<time.h>

void TermListener(int signal){
    if(signal!=SIGINT && signal!=SIGQUIT) return;
    printf("child: they want me to die...\n");
 }

void main(){
        __pid_t pid=fork();
        if(pid==0){
            signal(SIGINT, TermListener);
            signal(SIGQUIT, TermListener);
            time_t rtime;
            struct tm * timeinfo;
            char path[256]={0};
            sprintf(path,"out.txt");
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
            srand(time(NULL));
            for(int i=0;i<5;i++){
                sleep(3);
                int sig=rand()%2==0?SIGINT:SIGQUIT;
                printf("parent: sending %d\n",sig);
                kill(pid,sig);
                printf("parent: signal sent\n");
            }
            kill(pid, SIGKILL);
            wait(NULL);
        }
    
}