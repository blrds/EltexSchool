#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<signal.h>
#include<unistd.h>

#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/sem.h>

#define SHMEM_KEY "shmem_key"
#define SEM_MUTEX_KEY "sem_mutex_key"
#define PROJECT_ID 'b'


int whileFlag=1, parentReadyFlag=0, childReadyCounter=0;
pid_t childMINMAXPID, childAVERPID, childSUMPID, parentPID;

struct shared_memory{
    int mass[10];
    int count;
    int max, min, sum;
    double aver;
};

void INTListener(int signal){
    if(signal!=SIGINT)return;
    whileFlag=0;
    kill(childMINMAXPID,SIGINT);
    kill(childAVERPID, SIGINT);
    kill(childSUMPID, SIGINT);
}

void USR1Listener(int signal){
    if(signal!=SIGUSR1)return;
    parentReadyFlag=1;
}

void USR2Listener(int signal){
    if(signal!=SIGUSR2)return;
    childReadyCounter++;
}

void setop(int semid, int val, char who[], char what[]){
    struct sembuf asem[1];
    asem[0].sem_num=0;
	asem[0].sem_op=val;
	asem[0].sem_flg=0;
    if (semop (semid, asem, 1) == -1) {
        char str[256]={0};
        sprintf(str,"%s:semop: %s:", who, what);
        printf("A%dA\n",semctl(semid,1,GETVAL));
	    perror (str); 
         
    }
}

void setupsem(char* sem_key, char proj, int* sem, int setval, int val){
    key_t key;
    if ((key = ftok (sem_key, proj)) == -1) {
        perror ("ftok"); exit(1);
    }
    if (((*sem) = semget (key, 1, 0660 | IPC_CREAT)) == -1) {
        perror ("semget"); exit(1);
    }
    if(setval==1){
        union semun{
            int val;
            struct semid_ds *buf;
            ushort array[1];
        }sem_attr;
	    sem_attr.val = val;
        if (semctl ((*sem), 0, SETVAL, sem_attr) == -1) {
            perror ("semctl SETVAL"); exit(1);
        }
    }
}

struct shared_memory* setupshmem(char *shmem_key, char proj){
    key_t key;
    struct shared_memory* shmem_ptr;
    int shmem_id;
    if ((key = ftok (shmem_key, proj)) == -1) {
        perror ("ftok"); exit(1);
    }
    if ((shmem_id = shmget (key, sizeof(struct shared_memory), 0660 | IPC_CREAT)) == -1) {
        perror ("shmget"); exit(1);
    }
    if ((shmem_ptr=(struct shared_memory*)shmat(shmem_id, NULL, 0) )== (struct shared_memory*)-1) {
        perror ("shmat"); exit(1);
    }
    return shmem_ptr;
}

void childMINMAX(){
    struct shared_memory *shmem_ptr;
    int sem_mutex;
	setupsem(SEM_MUTEX_KEY,PROJECT_ID,&sem_mutex,0,0);
    shmem_ptr=setupshmem(SHMEM_KEY,PROJECT_ID);
    signal(SIGUSR1,USR1Listener);
    while(1){
        sleep(1);
        while(!parentReadyFlag)sleep(1);
        parentReadyFlag=0;
        setop(sem_mutex,-1,"childMINMAX","mutex");
        shmem_ptr->min=101;
        shmem_ptr->max=-1;
        for(int i=0;i<shmem_ptr->count;i++){
            if(shmem_ptr->min>shmem_ptr->mass[i])shmem_ptr->min=shmem_ptr->mass[i];
            if(shmem_ptr->max<shmem_ptr->mass[i])shmem_ptr->max=shmem_ptr->mass[i];
        }
        setop(sem_mutex,1,"childMINMAX","mutex");
        kill(parentPID,SIGUSR2);
    }
}

void childAVER(){
    struct shared_memory *shmem_ptr;
    int sem_mutex;
	setupsem(SEM_MUTEX_KEY,PROJECT_ID,&sem_mutex,0,0);
    shmem_ptr=setupshmem(SHMEM_KEY,PROJECT_ID);
    signal(SIGUSR1,USR1Listener);
    while(1){
        sleep(1);
        while(!parentReadyFlag)sleep(1);
        parentReadyFlag=0;
        setop(sem_mutex,-1,"childAVER","mutex");
        double sum=0.0l;
        for(int i=0;i<shmem_ptr->count;i++){
            sum+=shmem_ptr->mass[i];
        }
        shmem_ptr->aver=sum/shmem_ptr->count;
        setop(sem_mutex,1,"childAVER","mutex");
        kill(parentPID,SIGUSR2);
    }
}

void childSUM(){
    struct shared_memory *shmem_ptr;
    int sem_mutex;
	setupsem(SEM_MUTEX_KEY,PROJECT_ID,&sem_mutex,0,0);
    shmem_ptr=setupshmem(SHMEM_KEY,PROJECT_ID);
    signal(SIGUSR1,USR1Listener);
    while(1){
        sleep(1);
        while(!parentReadyFlag)sleep(1);
        parentReadyFlag=0;
        setop(sem_mutex,-1,"childSUM","mutex");
        int sum=0;
        for(int i=0;i<shmem_ptr->count;i++){
            sum+=shmem_ptr->mass[i];
        }
        shmem_ptr->sum=sum;
        setop(sem_mutex,1,"childSUM","mutex");
        kill(parentPID,SIGUSR2);
    }
}

void parent(){
    signal(SIGINT, INTListener);
    signal(SIGUSR2, USR2Listener);
    struct shared_memory *shmem_ptr;
    int sem_mutex;
	setupsem(SEM_MUTEX_KEY,PROJECT_ID,&sem_mutex,1,1);
////////////
    shmem_ptr=setupshmem(SHMEM_KEY,PROJECT_ID);
    shmem_ptr->max=0;
    shmem_ptr->min=0;
    shmem_ptr->count=0;
    shmem_ptr->sum=0;
    shmem_ptr->aver=0.0l;
    
    srand(time(NULL));
    int counter=0;
    while(whileFlag){
        sleep(1);
        setop(sem_mutex,-1,"parent","mutex");
        shmem_ptr->count=rand()%10+1;
        for(int i=0;i<10;i++){
            if(i<shmem_ptr->count)
                shmem_ptr->mass[i]=rand()%100;
            else shmem_ptr->mass[i]=0;
        }
        setop(sem_mutex,1,"parent","mutex");
        kill(childMINMAXPID, SIGUSR1);
        kill(childAVERPID, SIGUSR1);
        kill(childSUMPID, SIGUSR1);
        while(childReadyCounter<3){sleep(1);if(!whileFlag)break;}
        if(!whileFlag)break;
        childReadyCounter=0;
        setop(sem_mutex,-1,"parent","mutex");
        if(shmem_ptr->count>0 && shmem_ptr->max>=0 && shmem_ptr->min<100){
            printf("iteration %d: min=%d, max=%d, aver=%.2lf, sum=%d\n",
            counter,shmem_ptr->min, shmem_ptr->max, shmem_ptr->aver, shmem_ptr->sum);
            counter++;
        }
        setop(sem_mutex,1,"parent","mutex");
    }
    printf("total: %d\n",counter);
}

int main(){
    parentPID=getpid();
    childMINMAXPID=fork();
    if(childMINMAXPID==0){
        childMINMAX();
    }else{
        childAVERPID=fork();
        if(childAVERPID==0){
            childAVER();
        }else{
            childSUMPID=fork();
            if(childSUMPID==0){
                childSUM();
            }else{
                parent();
                wait(NULL);
                wait(NULL);
                wait(NULL);
            }
        }
    }
    exit(EXIT_SUCCESS);
}