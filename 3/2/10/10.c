#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<signal.h>
#include<time.h>
#include<mqueue.h>

#include<sys/types.h>

#define PATH "/mqueue\0"

int parent(){
    
    mqd_t ds;
    struct mq_attr queue_attr;
    queue_attr.mq_maxmsg=32;
    queue_attr.mq_msgsize=256;
    char str[256]={0};
    if((ds=mq_open(PATH,O_CREAT|O_RDWR, 0600, &queue_attr))==(mqd_t)-1){
        perror("p:mq_open");
        return EXIT_FAILURE;
    }
    srand(time(NULL));
    sleep(3);
    for(int i=0;i<5;i++){
        int a=rand()%20;
        sprintf(str,"%d\0",a);
        printf("parent generate %d\n",a);
        if(mq_send(ds,str,strlen(str),1)==-1){
            perror("p:mq_send");
            return EXIT_FAILURE;
        }
        sleep(1);
    }
    printf("parent send code 255\n");
    str[0]='\0';
    if(mq_send(ds,str,strlen(str),255)==-1){
            perror("p:mq_send");
            return EXIT_FAILURE;
        }
    if(mq_close(ds)==-1){
        perror("p:mq_close");
        return EXIT_FAILURE;
    }
}

int child(){
    mqd_t ds;
    struct mq_attr attr, old_attr;
    char str[256]={0};
    int prio;
    if((ds=mq_open(PATH,O_RDWR|O_NONBLOCK, 0600, NULL))==-1){
        perror("c:mq_open");
        return EXIT_FAILURE;
    }
    attr.mq_flags=0;
    if(mq_setattr(ds,&attr, NULL)){
        perror("c:mq_setattr");
        return EXIT_FAILURE;
    }
    if(mq_getattr(ds, &old_attr)){
        perror("c:mq_getattr");
        return EXIT_FAILURE;
    }
    while(1){
        sleep(1);
        memset(str,0,strlen(str));
        if(mq_receive(ds, str,256,&prio)==-1){
            perror("c:mq_receive");
            return EXIT_FAILURE;
        }
        if(prio==255)break;
        printf("child recieved:%s\n",str);
    }
    printf("child recieved code 255\n");
    if(mq_close(ds)==-1){
        perror("c:mq_close");
        return EXIT_FAILURE;
    }
    if(mq_unlink(PATH)==-1){
        perror("p:mq_unlink");
        return EXIT_FAILURE;
    }

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