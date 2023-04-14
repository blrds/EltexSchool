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
mqd_t qd_server;

typedef enum conStatus{
    con, dis
}conStatus;

typedef struct client
{
    conStatus status;
    char name[MAXNAME];
    char clientPath[MAXPATH];
}client;

void INTListener(int signal){
    if(signal!=SIGINT)return;
    whileFlag=0;
    if(mq_close(qd_server)==-1){
        perror("s:mq_close");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
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

void connect(char info[], client* clients, int *clientsCount){
    char *path=strtok(info,"|");
    char *name=strtok(NULL,"|");
    printf("%s connected\n",name);
    for(int i=0;i<(*clientsCount);i++){
        if(strcmp(clients[i].clientPath,path)==0){
            clients[i].status=con;
            strcpy(clients[i].name,name);
            return;
        }
    }
    clients=(client*)realloc(clients, ++(*clientsCount));
    strcpy(clients[(*clientsCount)-1].name,name);
    strcpy(clients[(*clientsCount)-1].clientPath,path);
    clients[(*clientsCount)-1].status=con;
}

void disconnect(char path[], client* clients, int clientsCount){
    for(int i=0;i<clientsCount;i++)
    {
        if(strcmp(clients[i].clientPath,path)==0){
            clients[i].status=dis;
            printf("%s disconnected\n",clients[i].name);
            return;
        }
    }
}

void resend(char* str, client* clients, int clientsCount){
    char name[MAXNAME]={0};
    char outstr[MAXSTR]={0};
    char *sender=strtok(str,"|");
    char *tmp=strtok(NULL,"|");
    for(int i=0;i<clientsCount;i++){
        if(strcmp(clients[i].clientPath,sender)==0){
            strcpy(name,clients[i].name);
            break;
        }
    }
    for(int i=0;i<clientsCount;i++){
        if(clients[i].status==dis)continue;
        if(strcmp(clients[i].clientPath,sender)==0){
            printf("%s:%s\n",name,tmp);
            continue;
        }
        sprintf(outstr,"%s:%s\0",name,tmp);
        if(outstr[strlen(outstr)-1]=='\n')outstr[strlen(outstr)-1]='\0';
        send(clients[i].clientPath,outstr,0);
    }
}

int main(){
    signal(SIGINT, INTListener);

    client *clients=(client*)malloc(sizeof(client));
    int clientsCount=0;
    char str[MAXSTR]={0};

    int prio;
    struct mq_attr queue_attr;
    queue_attr.mq_maxmsg=32;
    queue_attr.mq_msgsize=MAXSTR;

    if((qd_server=mq_open("/server",O_RDONLY|O_CREAT, 0660, &queue_attr))==(mqd_t)-1){
        perror("s:mq_open");
        exit(EXIT_FAILURE);
    }
    
    while(whileFlag){
        sleep(1);
        if(mq_receive(qd_server, str,MAXSTR,&prio)==-1){
            perror("s:mq_receive");
            exit(EXIT_FAILURE);
        }
        switch (prio){
        case 0:{
            resend(str,clients,clientsCount);
            break;
        }
        case 100:{
            connect(str,clients,&clientsCount);
            break;
        }
        case 255:{
            disconnect(str, clients, clientsCount);
            break;
        }
        default:
            break;
        }
    }
    if(mq_close(qd_server)==-1){
        perror("s:mq_close");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
    
}