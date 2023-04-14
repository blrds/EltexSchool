#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>
#include<string.h>
#include<fcntl.h>

#include<sys/types.h>
#include<sys/stat.h>
#include<sys/sem.h>
#include<sys/ipc.h>

#define SEM_MUTEX "sem_mutex"
#define SEM_SIGNAL "sem_signal"

int sem_mutex, sem_signal;

void setop(int semid, int val, char who[], char what[]){
    struct sembuf asem[1];
    asem[0].sem_num=0;
	asem[0].sem_op=val;
	asem[0].sem_flg=0;
    if (semop (semid, asem, 1) == -1) {
        char str[256]={0};
        sprintf(str,"%s:semop: %s:", who, what);
	    perror (str); 
        exit (1);
    }
}

void parent(int pipe,int count){

    for(int i=0;i<count;i++){
        sleep(3);
        printf("parent %d\n",i);
        int c;
        char str[256]={0};
        int i=0;
        memset(str, '\0', sizeof(str));
        read(pipe, str,sizeof(str));
        printf("parent red %s\n", str);
        setop(sem_mutex,-1,"parent","sem_mutex");
        FILE *f=fopen("a.txt", "w");
        fwrite(&str,sizeof(char),strlen(str),f);
        fclose(f);
        printf("parent wrote\n");
        setop(sem_mutex,1,"parent","sem_mutex");
        setop(sem_signal,1,"parent","sem_signal");
    }
}


void child(int pipe, int count){
    srand(time(NULL));
    int j=0;
    char str[256]={0};
    for(int i=0;i<count;i++){

        printf("child %d\n",i);
        j=rand()%20;
        memset(str,'\0',sizeof(str));
        sprintf(str,"%d%c",j,'\0');
        write(pipe, str, strlen(str));
        printf("child wrote to pipe %d\n",j);
        setop(sem_signal,-1,"child","sem_signal");
        setop(sem_mutex,-1,"child","sem_mutex");
        FILE *f=fopen("a.txt", "r");
        char str[256]={0};
        fgets(str,256,f);
        fclose(f);
        printf("Child red: %s\n",str);
        setop(sem_mutex,1,"child","sem_mutex");
    }
}

int main(int argc, char* argv[]){
    key_t key;
	union semun{
		int val;
		struct semid_ds *buf;
		ushort array[1];
	}sem_attr;
    if ((key = ftok (SEM_MUTEX, 'a')) == -1) {
        perror ("ftok"); exit (1);
    }
    if ((sem_mutex = semget (key, 1, 0660 | IPC_CREAT)) == -1) {
        perror ("semget"); exit (1);
    }
	sem_attr.val = 1;    
    if (semctl (sem_mutex, 0, SETVAL, sem_attr) == -1) {
        perror ("semctl SETVAL"); exit (1);
    }
    if ((key = ftok (SEM_SIGNAL, 'a')) == -1) {
        perror ("ftok"); exit (1);
    }
    if ((sem_signal = semget (key, 1, 0660 | IPC_CREAT)) == -1) {
        perror ("semget"); exit (1);
    }
	sem_attr.val = 0;    
    if (semctl (sem_signal, 0, SETVAL, sem_attr) == -1) {
        perror ("semctl SETVAL"); exit (1);
    }
    if(argc!=2)exit(EXIT_FAILURE);
    
    int _pipe[2];
    pipe(_pipe);
    if(fork()==0)child(_pipe[1],atoi(argv[1]));
    else {
        fcntl(_pipe[0],F_SETFL,fcntl(_pipe[0],F_GETFL)|O_NONBLOCK);
        parent(_pipe[0],atoi(argv[1]));
        wait(NULL);    
        if (semctl (sem_mutex, 0, IPC_RMID) == -1) {
            perror ("semctl IPC_RMID"); exit (1);
        }
        if (semctl (sem_signal, 0, IPC_RMID) == -1) {
            perror ("semctl IPC_RMID"); exit (1);
        }
    }
    exit(EXIT_SUCCESS);
}