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

void send(char serverPath[], char outstr[], int prior){
    mqd_t qd_server;
    if((qd_server=mq_open(serverPath,O_WRONLY))==(mqd_t)-1){
        perror("c->s:mq_open");
        exit(EXIT_FAILURE);
    }
    if(mq_send(qd_server,outstr,strlen(outstr)+1, prior)==-1){
        perror("c->s:mq_send");
        exit(EXIT_FAILURE);
    }
    if(mq_close(qd_server)==-1){
        perror("c->s:mq_close");
        exit(EXIT_FAILURE);
    }
}

int main(){
    char clientQN[64]={0};
    mqd_t qd_client;
    int prio;
    signal(SIGINT, INTListener);
    struct mq_attr queue_attr;
    queue_attr.mq_maxmsg=32;
    queue_attr.mq_msgsize=256;
    char instr[256]={0};
    char outstr[256]={0};
    sprintf(clientQN, "/qm%d",getpid());
    if((qd_client=mq_open(clientQN,O_RDONLY|O_CREAT, 0660, &queue_attr))==(mqd_t)-1){
        perror("c:mq_open");
        exit(EXIT_FAILURE);
    }
    send("/server",clientQN,0);
    while(whileFlag){
        fgets(instr,256,stdin);
        sprintf(outstr,"%s\0",instr);
        send("/server",outstr,0);
        sleep(1);
        if(whileFlag==0)break;
        if(mq_receive(qd_client, instr,256,&prio)==-1){
            perror("c->s:mq_receive");
            exit(EXIT_FAILURE);
        }
        if(prio==255){printf("() left\n");break;}
        printf("():%s\n",instr);
        
    }
    if(whileFlag==0){
        sprintf(outstr,"\0");
        send("/server",outstr,255);
    }
    if(mq_close(qd_client)==-1){
        perror("c:mq_close");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
    
}