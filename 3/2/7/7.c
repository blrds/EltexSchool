#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<signal.h>
#include<time.h>

#include<sys/ipc.h>
#include<sys/msg.h>
#include<sys/types.h>

#define PATH "mqueue"

typedef struct mText{
    int qid;
    char str[256];
}mText;


typedef struct message{
    int msg_type;
    mText text;
}message;

int parent(){
    sleep(3);
    key_t msgQKey;
    int cqid,mqid;
    if((msgQKey=ftok(PATH, 'M'))==-1){
        perror("p:ftok");
        return EXIT_FAILURE;
    }
    if((cqid=msgget(msgQKey, 0))==-1){
        perror("p:cmsgget");
        return EXIT_FAILURE;
    }
    if((mqid=msgget(IPC_PRIVATE,0660))==-1){
        perror("p:pmsgget");
        return EXIT_FAILURE;
    }
    for(int i=0;i<5;i++){
        srand(time(NULL));
        message msg;
        msg.msg_type=1;
        msg.text.qid=mqid;
        int a=rand()%20;
        sprintf(msg.text.str,"%d\0",a);
        printf("parent generate %d\n",a);
        if(msgsnd(cqid,&msg,sizeof(mText),0)==-1){
            perror("p:msgsnd");
            return EXIT_FAILURE;
        }
        sleep(1);
    }
    printf("parent send code 255\n");
    message msg;
    msg.msg_type=255;
    msg.text.qid=mqid;
    msg.text.str[0]='\0';
    if(msgsnd(cqid,&msg,sizeof(mText),0)==-1){
        perror("p:msgsnd");
        return EXIT_FAILURE;
    }
    if(msgctl(mqid,IPC_RMID, NULL)==-1){
        perror("p:msgctl");
        return EXIT_FAILURE;
    }
}

int child(){
    key_t msgQKey;
    int mqid;
    message msg;
    if((msgQKey=ftok(PATH, 'M'))==-1){
        perror("c:ftok");
        return EXIT_FAILURE;
    }
    if((mqid=msgget(msgQKey, IPC_CREAT|0660))==-1){
        perror("c:msgget");
        return EXIT_FAILURE;
    }
    while(1){
        sleep(1);
        if(msgrcv(mqid, &msg, sizeof(mText),0,0)==-1){
            perror("c:msgrcv");
            return EXIT_FAILURE;
        }
        if(msg.msg_type==255)break;
        printf("child recieved:%s\n",msg.text.str);
    }
    printf("child recieved code 255\n");
}

int main(){
    __pid_t pid=fork();
    int exStatus;
    if(pid==0)exit(child());
    else{
        exStatus=parent();
        if(exStatus==EXIT_FAILURE)kill(pid,SIGKILL);//for the error in parent, in purpose not to forget of orphan(
        wait(NULL);
    }
    exit(exStatus);
}