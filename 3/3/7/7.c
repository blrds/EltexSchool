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
#define SEM_CHILD_SIGNAL "sem_child_signal"
#define SEM_PARENT_SIGNAL "sem_parent_signal"
#define PROJECT_ID 'a'


int whileFlag=1;
pid_t childPID;

struct shared_memory{
    int mass[10];
    int count;
    int max, min;
};

void INTListener(int signal){
    if(signal!=SIGINT)return;
    whileFlag=0;
    kill(childPID,SIGINT);
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

void child(){
    key_t key;
    union semun{
        int val;
        struct semid_ds *buf;
        ushort array[1];
    }sem_attr;
    struct shared_memory *shmem_ptr;
    int sem_mutex,sem_child_signal, sem_parent_signal, shmem_id;
	if ((key = ftok (SEM_MUTEX_KEY, PROJECT_ID)) == -1) {
        perror ("ftok"); exit(1);
    }
    if ((sem_mutex = semget (key, 1, 0660 | IPC_CREAT)) == -1) {
        perror ("semget"); exit(1);
    }
////////////
    if ((key = ftok (SEM_CHILD_SIGNAL, PROJECT_ID)) == -1) {
        perror ("ftok"); exit(1);
    }
    if ((sem_child_signal = semget (key, 1, 0660 | IPC_CREAT)) == -1) {
        perror ("semget"); exit(1);
    }
	sem_attr.val = 1;
    if (semctl (sem_child_signal, 0, SETVAL, sem_attr) == -1) {
        perror ("semctl SETVAL"); exit(1);
    }
////////////
    if ((key = ftok (SEM_PARENT_SIGNAL, PROJECT_ID)) == -1) {
        perror ("ftok"); exit(1);
    }
    if ((sem_parent_signal = semget (key, 1, 0660 | IPC_CREAT)) == -1) {
        perror ("semget"); exit(1);
    }
//////////
    if ((key = ftok (SHMEM_KEY, PROJECT_ID)) == -1) {
        perror ("ftok"); exit(1);
    }
    if ((shmem_id = shmget (key, sizeof(struct shared_memory), 0660 | IPC_CREAT)) == -1) {
        perror ("shmget"); exit(1);
    }
    if ((shmem_ptr=(struct shared_memory*)shmat(shmem_id, NULL, 0) )== (struct shared_memory*)-1) {
        perror ("shmat"); exit(1);
    }
    shmem_ptr->max=shmem_ptr->min=shmem_ptr->count=0;

	sem_attr.val = 1;
    if (semctl (sem_mutex, 0, SETVAL, sem_attr) == -1) {
        perror ("semctl SETVAL"); exit(1);
    }
	sem_attr.val = 1; 
    if (semctl (sem_parent_signal, 0, SETVAL, sem_attr) == -1) {
        perror ("semctl SETVAL"); exit(1);
    }

    while(1){
        setop(sem_child_signal, -1, "child", "csignal");
        setop(sem_mutex,-1,"parent","mutex");
        shmem_ptr->min=101;
        shmem_ptr->max=-1;
        for(int i=0;i<shmem_ptr->count;i++){
            if(shmem_ptr->min>shmem_ptr->mass[i])shmem_ptr->min=shmem_ptr->mass[i];
            if(shmem_ptr->max<shmem_ptr->mass[i])shmem_ptr->max=shmem_ptr->mass[i];
        }
        setop(sem_mutex,1,"parent","mutex");
        setop(sem_parent_signal, 1, "child", "psignal");
    }
}

void parent(){
    signal(SIGINT, INTListener);
    key_t key;
    union semun{
        int val;
        struct semid_ds *buf;
        ushort array[1];
    }sem_attr;
    struct shared_memory *shmem_ptr;
    int sem_mutex,sem_child_signal, sem_parent_signal, shmem_id;
	if ((key = ftok (SEM_MUTEX_KEY, PROJECT_ID)) == -1) {
        perror ("ftok"); exit(1);
    }
    if ((sem_mutex = semget (key, 1, 0660 | IPC_CREAT)) == -1) {
        perror ("semget"); exit(1);
    }
////////////
    if ((key = ftok (SEM_CHILD_SIGNAL, PROJECT_ID)) == -1) {
        perror ("ftok"); exit(1);
    }
    if ((sem_child_signal = semget (key, 1, 0660 | IPC_CREAT)) == -1) {
        perror ("semget"); exit(1);
    }
////////////
    if ((key = ftok (SEM_PARENT_SIGNAL, PROJECT_ID)) == -1) {
        perror ("ftok"); exit(1);
    }
    if ((sem_parent_signal = semget (key, 1, 0660 | IPC_CREAT)) == -1) {
        perror ("semget"); exit(1);
    }
//////////
    if ((key = ftok (SHMEM_KEY, PROJECT_ID)) == -1) {
        perror ("ftok"); exit(1);
    }
    if ((shmem_id = shmget (key, sizeof(struct shared_memory), 0660 | IPC_CREAT)) == -1) {
        perror ("shmget"); exit(1);
    }
    if ((shmem_ptr=(struct shared_memory *)shmat(shmem_id, NULL, 0) )== (struct shared_memory*)-1) {
        perror ("shmat"); exit(1);
    }
    shmem_ptr->max=0;
    shmem_ptr->min=0;
    shmem_ptr->count=0;
    
    srand(time(NULL));
    int counter=0;
    while(whileFlag){
        sleep(1);
        setop(sem_parent_signal,-1,"parent","psignal");
        setop(sem_mutex,-1,"parent","mutex");
        shmem_ptr->count=rand()%10+1;
        for(int i=0;i<10;i++){
            if(i<shmem_ptr->count)
                shmem_ptr->mass[i]=rand()%100;
            else shmem_ptr->mass[i]=0;
        }
        setop(sem_mutex,1,"parent","mutex");
        setop(sem_child_signal,1,"parent","csignal");
        setop(sem_mutex,-1,"parent","mutex");
        if(shmem_ptr->count>0 && shmem_ptr->max>=0 && shmem_ptr->min<100){
        printf("iteration %d: min=%d, max=%d\n",counter,shmem_ptr->min, shmem_ptr->max);
        counter++;
        }
        setop(sem_mutex,1,"parent","mutex");
    }
    printf("total: %d\n",counter);
}

int main(){
    childPID=fork();
    if(childPID==0){
        child();
    }else{
        parent();
        wait(NULL);
    }
    exit(EXIT_SUCCESS);
}