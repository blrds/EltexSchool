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
#define SIZE sizeof(shared_memory)

sem_t mutex, cSignal, pSignal;
int whileFlag=1;
pthread_t p, c;
int counter;

typedef struct shared_memory{
    int mass[10];
    int count;
    int max, min;
}shared_memory;

void PINTListener(int signal){
    if(signal!=SIGINT)return;
    pthread_kill(c, SIGINT);
    printf("total: %d\n",counter);
    shm_unlink(SHMEM_KEY);
    exit(EXIT_SUCCESS);
}

void MINTListener(int signal){
    if(signal!=SIGINT)return;
    pthread_kill(p, SIGINT);
}

void error(char* mes){
    perror(mes);
    exit(1);
}

void* child(void* arg){
    sleep(1);
    shared_memory *shmem_ptr;
    int shmem_id;
    if((shmem_id=shm_open(SHMEM_KEY, O_RDWR, 0660))<0)error("CHILD:shm_open");


    while(1){
        if((sem_wait(&cSignal))==-1)error("child:sem_wait:cSignal");
        if((sem_wait(&mutex))==-1)error("child:sem_wait:mutex");
        if((shmem_ptr=(shared_memory*)mmap(0,SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,shmem_id,0))==(shared_memory*)-1)error("child:mmap");
        shmem_ptr->min=101;
        shmem_ptr->max=-1;
        for(int i=0;i<shmem_ptr->count;i++){
            if(shmem_ptr->min>shmem_ptr->mass[i])shmem_ptr->min=shmem_ptr->mass[i];
            if(shmem_ptr->max<shmem_ptr->mass[i])shmem_ptr->max=shmem_ptr->mass[i];
        }
        munmap(shmem_ptr, SIZE);
        if((sem_post(&mutex))==-1)error("child:sem_post:mutex");
        if((sem_post(&pSignal))==-1)error("CHILD:sem_post:pSignal");
    }
}

void* parent(void* arg){
    signal(SIGINT, PINTListener);
    int shmem_id;
    shared_memory *shmem_ptr=NULL;
    if((shmem_id=shm_open(SHMEM_KEY, O_CREAT|O_RDWR, 0660))<0)error("parent:shm_open");
    ftruncate(shmem_id, SIZE);
    if((shmem_ptr=(shared_memory*)mmap(0,SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,shmem_id,0))==(shared_memory*)-1)error("child:mmap");
    shmem_ptr->count=0;
    shmem_ptr->min=0;
    shmem_ptr->max=0;
    munmap(shmem_ptr, SIZE);
    srand(time(NULL));
    counter=0;
    while(whileFlag){
        sleep(1);
        if((sem_wait(&mutex))==-1)error("parent:sem_wait:mutex");
        if((shmem_ptr=(shared_memory*)mmap(0,SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,shmem_id,0))==(shared_memory*)-1)error("child:mmap");
        shmem_ptr->count=rand()%10+1;
        for(int i=0;i<10;i++){
            if(i<shmem_ptr->count)
                shmem_ptr->mass[i]=rand()%100;
            else shmem_ptr->mass[i]=0;
        }
        munmap(shmem_ptr, SIZE);
        if((sem_post(&mutex))==-1)error("parent:sem_post:mutex");
        if((sem_post(&cSignal))==-1)error("parent:sem_post:cSignal");
        if((sem_wait(&pSignal))==-1)error("parent:sem_wait:pSignal");
        if((sem_wait(&mutex))==-1)error("parent:sem_wait:mutex");
        shmem_ptr=(shared_memory*)mmap(0,SIZE,PROT_READ,MAP_SHARED,shmem_id,0);
        if(shmem_ptr->count>0 && shmem_ptr->max>=0 && shmem_ptr->min<100){
            printf("iteration %d: min=%d, max=%d\n",counter,shmem_ptr->min, shmem_ptr->max);
            counter++;
        }
        munmap(shmem_ptr, SIZE);
        if((sem_post(&mutex))==-1)error("parent:sem_post:mutex");
    }
}

int main(){
    sem_init(&mutex,0,1);
    sem_init(&cSignal,0,0);
    sem_init(&pSignal,0,0);
    pthread_create(&p,NULL, parent,NULL);
    pthread_create(&c,NULL, child,NULL);
    pthread_join(c, NULL);
    pthread_join(p, NULL);
    sem_destroy(&mutex);
    sem_destroy(&cSignal);
    sem_destroy(&pSignal);
    exit(EXIT_SUCCESS);
}