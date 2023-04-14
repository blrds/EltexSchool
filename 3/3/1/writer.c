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

#define SEM_WRITER_MUTEX "sem_writer_mutex"

int main(int argc, char* argv[]){
	key_t key;
	union semun{
		int val;
		struct semid_ds *buf;
		ushort array[1];
	}sem_attr;
	int sem_writer_mutex;
	if ((key = ftok (SEM_WRITER_MUTEX, 'a')) == -1) {
        perror ("ftok"); exit (1);
    }
    if ((sem_writer_mutex = semget (key, 1, 0660 | IPC_CREAT)) == -1) {
        perror ("semget"); exit (1);
    }
	sem_attr.val = 1;    
    if (semctl (sem_writer_mutex, 0, SETVAL, sem_attr) == -1) {
        perror ("semctl SETVAL"); exit (1);
    }
	
    srand(time(NULL));
    char fifoFile[]="fifo0001.1";
    int fifo;
    char str[80]={0};
    if(argc==2 && atoi(argv[1])==-1)unlink(fifoFile);
    mkfifo(fifoFile,O_RDWR);
	
	struct sembuf asem[1];
	pid_t pid=fork();
	int prInt=rand()%10+(int)pid;
	asem[0].sem_num=0;
	asem[0].sem_op=0;
	asem[0].sem_flg=0;
    while(1){
		asem[0].sem_op=-1;
		if (semop (sem_writer_mutex, asem, 1) == -1) {//второй поток стоит и ждет, пока первый напишет свои 3 чисал, потом меняются
			perror ("semop: sem_writer_mutex"); exit (1);
        }
		for(int i=0;i<3;i++){
			sleep(3);
			if((fifo=open(fifoFile,O_WRONLY))==-1){
				perror("open");
				exit(EXIT_FAILURE);
			}
			   sprintf(str,"%d\0",prInt);
			   write(fifo,str,strlen(str));
			   printf("%s\n", str);
			close(fifo);
		}
		asem[0].sem_op=1;
		if (semop (sem_writer_mutex, asem, 1) == -1) {
			perror ("semop: sem_writer_mutex"); exit (1);
        }
    }
	if(pid!=0){
		wait(NULL);
		unlink(fifoFile);
	if (semctl (sem_writer_mutex, 0, IPC_RMID) == -1) {
        perror ("semctl IPC_RMID"); exit (1);
    }
		}
    exit(EXIT_SUCCESS);
}