#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<signal.h>
#include<time.h>
#include<mqueue.h>

#include<sys/types.h>

int whileFlag=1;

void INTListener(int signal){
    if(signal!=SIGINT)return;
    whileFlag=0;
}

void send(char clientPath[], char outstr[], int prior){
    mqd_t qd_client;
    if((qd_client=mq_open(clientPath,O_WRONLY))==(mqd_t)-1){
        perror("s->c:mq_open");
        exit(EXIT_FAILURE);
    }
    if(mq_send(qd_client,outstr,strlen(outstr)+1, prior)==-1){
        perror("s->c:mq_send");
        exit(EXIT_FAILURE);
    }
    if(mq_close(qd_client)==-1){
        perror("s->c:mq_close");
        exit(EXIT_FAILURE);
    }
}

int main(){
    mqd_t qd_server;
    int prio;
    signal(SIGINT, INTListener);
    struct mq_attr queue_attr;
    queue_attr.mq_maxmsg=32;
    queue_attr.mq_msgsize=256;
    if((qd_server=mq_open("/server",O_RDONLY|O_CREAT, 0660, &queue_attr))==(mqd_t)-1){
        perror("s:mq_open");
        exit(EXIT_FAILURE);
    }
    char instr[256]={0};
    char outstr[256]={0};
    char clientPath[256]={0};
    if(mq_receive(qd_server, clientPath,256,&prio)==-1){
        perror("s->c:mq_receive");
        exit(EXIT_FAILURE);
    }
    while(whileFlag){
        sleep(1);
        if(mq_receive(qd_server, instr,256,&prio)==-1){
            perror("s:mq_receive");
            exit(EXIT_FAILURE);
        }
        if(prio==255){printf("() left\n");break;}
        printf("():%s\n",instr);
        if(whileFlag)
        fgets(instr,256,stdin);      
        sprintf(outstr,"%s\0",instr);
        send(clientPath,outstr,0);
    }
    if(whileFlag==0){
        sprintf(outstr,"\0");
        send(clientPath,outstr,255);
    }
    if(mq_close(qd_server)==-1){
        perror("s:mq_close");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
    
}