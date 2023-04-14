#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>
#include<errno.h>
#include<semaphore.h>
#include<pthread.h>

#include<sys/types.h>
#include<sys/stat.h>
#include<sys/ipc.h>

sem_t reader_mutex;
int fifo;
char fifoFile[]="fifo0001.1";

void* func(void* arg){
    char str[80]={0};
	while(1){
		sem_wait(&reader_mutex);
		for(int i=0;i<3;i++){
			sleep(1);
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
				printf("%d:%s\n",gettid(),str);
			close(fifo);
		}
		sem_post(&reader_mutex);
		sleep(1);
    }
}

int main(){
	sem_init(&reader_mutex,1,1);
	pthread_t t1,t2;
	pthread_create(&t1,NULL, func, NULL);
	sleep(1);
	pthread_create(&t2,NULL, func, NULL);
	pthread_join(t1,NULL);
	pthread_join(t2,NULL);
	sem_destroy(&reader_mutex);
    exit(EXIT_SUCCESS);
}