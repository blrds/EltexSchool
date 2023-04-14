#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>
#include<errno.h>

#include<sys/types.h>
#include<sys/stat.h>
#include<sys/sem.h>
#include<sys/ipc.h>

#define SEM_READER_MUTEX "sem_reader_mutex"

int main(){
	key_t key;
	union semun{
		int val;
		struct semid_ds *buf;
		ushort array[1];
	}sem_attr;
	int sem_reader_mutex;
	if ((key = ftok (SEM_READER_MUTEX, 'a')) == -1) {
        perror ("ftok"); exit (1);
    }
    if ((sem_reader_mutex = semget (key, 1, 0660 | IPC_CREAT)) == -1) {
        perror ("semget"); exit (1);
    }
	sem_attr.val = 1;    
    if (semctl (sem_reader_mutex, 0, SETVAL, sem_attr) == -1) {
        perror ("semctl SETVAL"); exit (1);
    }
    int fifo;
    char fifoFile[]="fifo0001.1";
    char str[80]={0};
	struct sembuf asem[1];
	pid_t pid=fork();
	asem[0].sem_num=0;
	asem[0].sem_op=0;
	asem[0].sem_flg=0;
    while(1){
		asem[0].sem_op=-1;
		if (semop (sem_reader_mutex, asem, 1) == -1) {//второй поток стоит и ждет, пока первый считает свои 3 чисал, потом меняются
			perror ("semop: sem_reader_mutex1"); exit (1);
        }
		for(int i=0;i<3;i++){
			if((fifo=open(fifoFile,O_RDWR))==-1){
				perror("open");
				exit(EXIT_FAILURE);
			}
			sprintf(str,"\0");
			memset(str,'\0',sizeof(str));
			if((read(fifo,&str, sizeof(str)))==-1){
				perror("read");
				exit(EXIT_FAILURE);
			}else
				printf("%d:%s\n",pid,str);
			close(fifo);
		}
		asem[0].sem_op=1;
		if (semop (sem_reader_mutex, asem, 1) == -1) {
			perror ("semop: sem_reader_mutex2"); exit (1);
        }
    }
	if(pid!=0){
		wait(NULL);
	if (semctl (sem_reader_mutex, 0, IPC_RMID) == -1) {
        perror ("semctl IPC_RMID"); exit (1);
    }
	}
    exit(EXIT_SUCCESS);
}