#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<signal.h>
#include<time.h>

#include<sys/ipc.h>
#include<sys/msg.h>
#include<sys/types.h>

int whileFlag=1;
int qid,sqid;
__pid_t cpid;

typedef struct mText{
    int qid;
    char str[1065];
}mText;


typedef struct message{
    int msg_type;
    mText text;
}message;

void INTListener(int signal){
    if(signal!=SIGINT)return;
    whileFlag=0;
    kill(cpid,SIGINT);
}

void recieve(){
    message msg;
    while(1){
        if(msgrcv(qid, &msg, sizeof(mText),0,0)==-1){
            perror("c:msgrcv");
            exit(EXIT_FAILURE);
        }
        printf("%s\n",msg.text.str);
        sleep(1);
    }
}

int main(){
    key_t msgQKey;
    message msg;
    char str[1024]={0};
    if((msgQKey=ftok("mqueue", 'M'))==-1){
        perror("c:ftok");
        exit(EXIT_FAILURE);
    }
    if((sqid=msgget(msgQKey, 0))==-1){
        perror("c:cmsgget");
        exit(EXIT_FAILURE);
    }
    if((qid=msgget(IPC_PRIVATE,0660))==-1){
        perror("c:pmsgget");
        exit(EXIT_FAILURE);
    }
    printf("Name:\n");
    fgets(str,40,stdin);
    msg.msg_type=100;
    msg.text.qid=qid;
    sprintf(msg.text.str,"%s\0",str);
    if(msg.text.str[strlen(msg.text.str)-1]=='\n')msg.text.str[strlen(msg.text.str)-1]='\0';
    if(msgsnd(sqid,&msg,sizeof(mText),0)==-1){
        perror("c:msgsnd");
        exit(EXIT_FAILURE);
    }
    cpid=fork();
    if(cpid==0)recieve();
    else {
        signal(SIGINT, INTListener);
        while(whileFlag){
            msg.msg_type=0;
            fgets(str,1024,stdin);
            sprintf(msg.text.str,"%s\0",str);
            if(msg.text.str[strlen(msg.text.str)-1]=='\n')msg.text.str[strlen(msg.text.str)-1]='\0';
            if(msgsnd(sqid,&msg,sizeof(mText),0)==-1){
                perror("s:msgsnd");
                exit(EXIT_FAILURE);
            }
            sleep(1);
        }
        msg.msg_type=255;
        sprintf(msg.text.str,"\0");
        if(msgsnd(sqid,&msg,sizeof(mText),0)==-1){
            perror("s:msgsnd");
            exit(EXIT_FAILURE);
        }
    }
    exit(EXIT_SUCCESS);
}