#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<signal.h>
#include<time.h>
#include<mqueue.h>

#include<sys/types.h>

#define MAXMSG 1024
#define MAXNAME 32
#define MAXPATH 64
#define MAXSTR MAXMSG+MAXNAME+MAXPATH

int whileFlag=1;
__pid_t cpid;

void send(char recieverPath[], char outstr[], int prior){
    mqd_t qd_reciever;
    if((qd_reciever=mq_open(recieverPath,O_WRONLY))==(mqd_t)-1){
        perror("s->c:mq_open");
        exit(EXIT_FAILURE);
    }
    if(mq_send(qd_reciever,outstr,strlen(outstr)+1, prior)==-1){
        perror("s->c:mq_send");
        exit(EXIT_FAILURE);
    }
    if(mq_close(qd_reciever)==-1){
        perror("s->c:mq_close");
        exit(EXIT_FAILURE);
    }
}

void INTListener(int signal){
    if(signal!=SIGINT)return;
    whileFlag=0;
    kill(cpid,SIGINT);
}

void recieve(struct mq_attr attr, char clientQN[]){
    char instr[MAXSTR]={0};
    int prio;
    mqd_t qd_client;
    if((qd_client=mq_open(clientQN,O_RDONLY|O_CREAT, 0660, &attr))==(mqd_t)-1){
        perror("c:mq_open");
        exit(EXIT_FAILURE);
    }
    while(1){
        if(mq_receive(qd_client, instr,MAXSTR,&prio)==-1){
            perror("c->s:mq_receive");
            exit(EXIT_FAILURE);
        }
        printf("%s\n",instr);
        sleep(1);
    }
    if(mq_close(qd_client)==-1){
        perror("c:mq_close");
        exit(EXIT_FAILURE);
    }
}

int main(){
    char str[MAXMSG]={0};
    char outstr[MAXSTR]={0};
    char clientQN[MAXPATH]={0};

    struct mq_attr queue_attr;
    queue_attr.mq_maxmsg=32;
    queue_attr.mq_msgsize=1065;


    sprintf(clientQN, "/qm%d",getpid());

    printf("Name:\n");
    fgets(str,MAXNAME,stdin);
    sprintf(outstr,"%s|%s\0",clientQN,str);
    if(outstr[strlen(outstr)-1]=='\n')outstr[strlen(outstr)-1]='\0';
    send("/server",outstr,100);

    cpid=fork();
    if(cpid==0)recieve(queue_attr,clientQN);
    else {
        signal(SIGINT, INTListener);
        while(whileFlag){
            fgets(str,MAXMSG,stdin);
            sprintf(outstr,"%s|%s\0",clientQN,str);
            if(outstr[strlen(outstr)-1]=='\n')outstr[strlen(outstr)-1]='\0';
            send("/server",outstr,0);
            sleep(1);
        }
        sprintf(outstr,"%s\0",clientQN);
        send("/server",outstr,255);
    }
    exit(EXIT_SUCCESS);
}