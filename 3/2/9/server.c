#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<signal.h>
#include<time.h>
#include<malloc.h>

#include<sys/ipc.h>
#include<sys/msg.h>
#include<sys/types.h>

int whileFlag=1;

typedef struct mText{
    int qid;
    char str[1065];
}mText;


typedef struct message{
    int msg_type;
    mText text;
}message;

typedef enum conStatus{
    con, dis
}conStatus;

typedef struct client
{
    int cqid;
    conStatus status;
    char name[40];
}client;

void connect(int cqid, char name[], client* clients, int *clientsCount){
    printf("%s connected\n",name);
    for(int i=0;i<(*clientsCount);i++){
        if(clients[i].cqid==cqid){
            clients[i].status=con;
            strcpy(clients[i].name,name);
            return;
        }
    }
    clients=(client*)realloc(clients, ++(*clientsCount));
    clients[(*clientsCount)-1].cqid=cqid;
    strcpy(clients[(*clientsCount)-1].name,name);
    clients[(*clientsCount)-1].status=con;
}

void disconnect(int cqid, client* clients, int clientsCount){
    for(int i=0;i<clientsCount;i++)
    {
        if(clients[i].cqid==cqid){
            clients[i].status=dis;
            return;
        }
    }
}

void resend(char* str, client* clients, int clientsCount, int qid, int senderQID){
    message msg;
    msg.msg_type=0;
    msg.text.qid=qid;
    char name[40];
    for(int i=0;i<clientsCount;i++){
        if(clients[i].cqid==senderQID)strcpy(name,clients[i].name);
        break;
    }
    for(int i=0;i<clientsCount;i++){
        if(clients[i].status==dis)continue;
        if(clients[i].cqid==senderQID){
            printf("%s:%s\n",name,str);
            continue;
        }
        sprintf(msg.text.str,"%s:%s\0",name,str);
        if(msg.text.str[strlen(msg.text.str)-1]=='\n')msg.text.str[strlen(msg.text.str)-1]='\0';
        if(msgsnd(clients[i].cqid,&msg,sizeof(mText),0)==-1){
            perror("s:msgsnd");
            return EXIT_FAILURE;
        }   
    }
}

int main(){
    key_t msgQKey;
    message msg;
    int qid,cqid, clientsCount=0;
    client* clients=(client*)malloc(sizeof(client));
    if((msgQKey=ftok("mqueue", 'M'))==-1){
        perror("s:ftok");
        exit(EXIT_FAILURE);
    }
    if((qid=msgget(msgQKey, IPC_CREAT|0660))==-1){
        perror("s:msgget");
        exit(EXIT_FAILURE);
    }
    char str[512]={0};
    while(whileFlag){
        sleep(1);
        if(msgrcv(qid, &msg, sizeof(mText),0,0)==-1){
            perror("s:msgrcv");
            return EXIT_FAILURE;
        }
        if(msg.msg_type==100){
            connect(msg.text.qid,msg.text.str,clients,&clientsCount);
            continue;
        }
        if(msg.msg_type==255){
            disconnect(msg.text.qid,clients,clientsCount);
            continue;
        }
        if(msg.msg_type==0){
            resend(msg.text.str,clients,clientsCount,qid, msg.text.qid);
            continue;
        }
    }
    
    exit(EXIT_SUCCESS);
    
}