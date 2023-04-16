#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<signal.h>
#include<unistd.h>
#include<semaphore.h>
#include<pthread.h>
#include<fcntl.h>

#include<sys/types.h>
#include<sys/mman.h>

#define SHMEM_KEY "shmem_key"
#define SIZE sizeof(shmem)


int whileFlag=1, parentReadyFlag=0, childReadyCounter=0;
pthread_t p, cMINMAX, cAVER, cSUM;
sem_t mutex;

typedef struct shared_memory{
    int mass[10];
    int count;
    int max, min, sum;
    double aver;
}shmem;

void PINTListener(int signal){
    if(signal!=SIGINT)return;
    whileFlag=0;
    pthread_kill(cMINMAX,SIGINT);
    pthread_kill(cAVER, SIGINT);
    pthread_kill(cSUM, SIGINT);
}

void MINTListener(int signal){
    if(signal!=SIGINT)return;
    pthread_kill(p, SIGINT);
}

void USR1Listener(int signal){
    if(signal!=SIGUSR1)return;
    parentReadyFlag=1;
}

void USR2Listener(int signal){
    if(signal!=SIGUSR2)return;
    childReadyCounter++;
}

void error(char* mes){
    perror(mes);
    exit(1);
}

void* childMINMAX(void* arg){
    signal(SIGUSR1,USR1Listener);
    shmem *shmem_ptr;
    int shmem_id;
    if((shmem_id=shm_open(SHMEM_KEY, O_RDWR, 0660))<0)error("childMINMAX:shm_open");
    while(1){
        sleep(1);
        while(!parentReadyFlag)sleep(1);
        parentReadyFlag=0;
        if((sem_wait(&mutex))==-1)error("childMINMAX:sem_wait:mutex");
        if((shmem_ptr=(shmem*)mmap(0,SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,shmem_id,0))==(shmem*)-1)error("childMINMAX:mmap");
        shmem_ptr->min=101;
        shmem_ptr->max=-1;
        for(int i=0;i<shmem_ptr->count;i++){
            if(shmem_ptr->min>shmem_ptr->mass[i])shmem_ptr->min=shmem_ptr->mass[i];
            if(shmem_ptr->max<shmem_ptr->mass[i])shmem_ptr->max=shmem_ptr->mass[i];
        }
        munmap(shmem_ptr, SIZE);
        if((sem_post(&mutex))==-1)error("childMINMAX:sem_post:mutex");
        pthread_kill(p,SIGUSR2);
    }
}

void* childAVER(void* arg){
    signal(SIGUSR1,USR1Listener);
    shmem *shmem_ptr;
    int shmem_id;
    if((shmem_id=shm_open(SHMEM_KEY, O_RDWR, 0660))<0)error("childAVER:shm_open");
    while(1){
        sleep(1);
        while(!parentReadyFlag)sleep(1);
        parentReadyFlag=0;
        if((sem_wait(&mutex))==-1)error("childAVER:sem_wait:mutex");
        if((shmem_ptr=(shmem*)mmap(0,SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,shmem_id,0))==(shmem*)-1)error("childAVER:mmap");
        double sum=0.0l;
        for(int i=0;i<shmem_ptr->count;i++){
            sum+=shmem_ptr->mass[i];
        }
        shmem_ptr->aver=sum/shmem_ptr->count;
        munmap(shmem_ptr, SIZE);
        if((sem_post(&mutex))==-1)error("childAVER:sem_post:mutex");
        pthread_kill(p,SIGUSR2);
    }
}

void* childSUM(void* arg){
    signal(SIGUSR1,USR1Listener);
    shmem *shmem_ptr;
    int shmem_id;
    if((shmem_id=shm_open(SHMEM_KEY, O_RDWR, 0660))<0)error("childSUM:shm_open");
    while(1){
        sleep(1);
        while(!parentReadyFlag)sleep(1);
        parentReadyFlag=0;
        if((sem_wait(&mutex))==-1)error("childSUM:sem_wait:mutex");
        if((shmem_ptr=(shmem*)mmap(0,SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,shmem_id,0))==(shmem*)-1)error("childSUM:mmap");
        int sum=0;
        for(int i=0;i<shmem_ptr->count;i++){
            sum+=shmem_ptr->mass[i];
        }
        shmem_ptr->sum=sum;
        munmap(shmem_ptr, SIZE);
        if((sem_post(&mutex))==-1)error("childSUM:sem_post:mutex");
        pthread_kill(p,SIGUSR2);
    }
}

void* parent(void* arg){
    signal(SIGINT, PINTListener);
    signal(SIGUSR2, USR2Listener);
    pthread_create(&cMINMAX,NULL,childMINMAX,NULL);
    pthread_create(&cAVER,NULL,childAVER,NULL);
    pthread_create(&cSUM,NULL,childSUM,NULL);
    int shmem_id;
    shmem *shmem_ptr=NULL;
    if((shmem_id=shm_open(SHMEM_KEY, O_CREAT|O_RDWR, 0660))<0)error("parent:shm_open");
    ftruncate(shmem_id, SIZE);
    if((shmem_ptr=(shmem*)mmap(0,SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,shmem_id,0))==(shmem*)-1)error("child:mmap");
    shmem_ptr->max=0;
    shmem_ptr->min=0;
    shmem_ptr->count=0;
    shmem_ptr->sum=0;
    shmem_ptr->aver=0.0l;
    munmap(shmem_ptr, SIZE);
    srand(time(NULL));
    int counter=0;
    while(whileFlag){
        sleep(1);
        if((sem_wait(&mutex))==-1)error("parent:sem_wait:mutex");
        if((shmem_ptr=(shmem*)mmap(0,SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,shmem_id,0))==(shmem*)-1)error("child:mmap");
        shmem_ptr->count=rand()%10+1;
        for(int i=0;i<10;i++){
            if(i<shmem_ptr->count)
                shmem_ptr->mass[i]=rand()%100;
            else shmem_ptr->mass[i]=0;
        }
        munmap(shmem_ptr, SIZE);
        if((sem_post(&mutex))==-1)error("parent:sem_post:mutex");
        pthread_kill(cMINMAX, SIGUSR1);
        pthread_kill(cAVER, SIGUSR1);
        pthread_kill(cSUM, SIGUSR1);
        while(childReadyCounter<3){sleep(1);if(!whileFlag)break;}
        if(!whileFlag)break;
        childReadyCounter=0;
        if((sem_wait(&mutex))==-1)error("parent:sem_wait:mutex");
        if((shmem_ptr=(shmem*)mmap(0,SIZE,PROT_READ,MAP_SHARED,shmem_id,0))==(shmem*)-1)error("child:mmap");
        if(shmem_ptr->count>0 && shmem_ptr->max>=0 && shmem_ptr->min<100){
            printf("iteration %d: min=%d, max=%d, aver=%.2lf, sum=%d\n",
            counter,shmem_ptr->min, shmem_ptr->max, shmem_ptr->aver, shmem_ptr->sum);
            counter++;
        }
        munmap(shmem_ptr, SIZE);
        if((sem_post(&mutex))==-1)error("parent:sem_post:mutex");
    }
    printf("total: %d\n",counter);
    pthread_join(cMINMAX, NULL);
    printf("cMM: %d\n",counter);
    pthread_join(cAVER, NULL);
    printf("cA: %d\n",counter);
    pthread_join(cSUM, NULL);
    printf("cS: %d\n",counter);
    shm_unlink(SHMEM_KEY);
}

int main(){
    signal(SIGINT, MINTListener);
    sem_init(&mutex,0,1);
    pthread_create(&p,NULL, parent, NULL);
    pthread_join(p, NULL);
    sem_destroy(&mutex);
    exit(EXIT_SUCCESS);
}