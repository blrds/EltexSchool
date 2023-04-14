#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>
#include<string.h>
#include<fcntl.h>
#include<semaphore.h>
#include<errno.h>
#include<pthread.h>

#include<sys/types.h>
#include<sys/stat.h>
#include<sys/ipc.h>

sem_t writer_mutex;
int fifo;
char fifoFile[]="fifo0001.1";

void* func(void* arg){
    char str[80]={0};
    srand(time(NULL));
	int prInt=rand()%1000;
	while(1){
		sleep(1);
		sem_wait(&writer_mutex);
		for(int i=0;i<3;i++){
			sleep(1);
			if((fifo=open(fifoFile,O_WRONLY))==-1){
				perror("open");
				exit(EXIT_FAILURE);
			}
			memset(str,'\0',sizeof(str));
			sprintf(str,"%d\0",prInt);
			write(fifo,str,strlen(str));
			printf("%d:%s\n",gettid(),str);
			close(fifo);
		}
		sem_post(&writer_mutex);
    }
}

int main(int argc, char* argv[]){
	sem_init(&writer_mutex,0,1);
    unlink(fifoFile);
    mkfifo(fifoFile,O_RDWR);
	pthread_t t1,t2;
	pthread_create(&t1, NULL, func,NULL);
	sleep(1);
	pthread_create(&t2, NULL, func,NULL);
	pthread_join(t1,NULL);
	pthread_join(t2,NULL);	
	sem_destroy(&writer_mutex);
    exit(EXIT_SUCCESS);
}