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

typedef struct mText{
    int qid;
    char str[256];
}mText;


typedef struct message{
    int msg_type;
    mText text;
}message;

void INTListener(int signal){
    if(signal!=SIGINT)return;
    whileFlag=0;
}
int main(){
    key_t msgQKey;
    message msg;
    int qid,cqid;

    signal(SIGINT, INTListener);
    if((msgQKey=ftok("mqueue", 'M'))==-1){
        perror("s:ftok");
        exit(EXIT_FAILURE);
    }
    if((qid=msgget(msgQKey, IPC_CREAT|0660))==-1){
        perror("s:msgget");
        exit(EXIT_FAILURE);
    }
    char str[256]={0};
    while(whileFlag){
        sleep(1);
        if(msgrcv(qid, &msg, sizeof(mText),0,0)==-1){
            perror("s:msgrcv");
            return EXIT_FAILURE;
        }
        if(msg.msg_type==255){printf("() left\n");break;}
        printf("():%s\n",msg.text.str);
        if(whileFlag)
        fgets(str,256,stdin);
        cqid=msg.text.qid;
        msg.msg_type=0;
        msg.text.qid=qid;
        sprintf(msg.text.str,"%s\0",str);
        if(msgsnd(cqid,&msg,sizeof(mText),0)==-1){
            perror("s:msgsnd");
            return EXIT_FAILURE;
        }
    }
    if(whileFlag==0){
        msg.msg_type=255;
        sprintf(msg.text.str,"\0");
        msg.text.qid=qid;
        if(msgsnd(cqid,&msg,sizeof(mText),0)==-1){
            perror("s:msgsnd");
            return EXIT_FAILURE;
        }
    }
    exit(EXIT_SUCCESS);
    
}