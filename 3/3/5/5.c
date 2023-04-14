#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>
#include<string.h>
#include<fcntl.h>
#include<pthread.h>
#include<semaphore.h>

#include<sys/types.h>
#include<sys/stat.h>

sem_t mutex, signal;
int _pipe[2], count;

void* parent(void *arg){
    int pipe=_pipe[0];
    for(int i=0;i<count;i++){
        sleep(1);
        printf("parent %d\n",i);
        int c;
        char str[256]={0};
        int i=0;
        memset(str, '\0', sizeof(str));
        read(pipe, str,sizeof(str));
        printf("parent red %s\n", str);
        sem_wait(&mutex);
        FILE *f=fopen("a.txt", "w");
        fwrite(&str,sizeof(char),strlen(str),f);
        fclose(f);
        printf("parent wrote\n");
        sem_post(&mutex);
        sem_post(&signal);
    }
}


void* child(void* arg){
    int pipe=_pipe[1];
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
        sem_wait(&signal);
        sem_wait(&mutex);
        FILE *f=fopen("a.txt", "r");
        char str[256]={0};
        fgets(str,256,f);
        fclose(f);
        printf("Child red: %s\n",str);
        sem_post(&mutex);
    }
}

int main(int argc, char* argv[]){
    sem_init(&mutex,0,1);
    sem_init(&signal,0,0);
    if(argc!=2)exit(EXIT_FAILURE);
    pipe(_pipe);
    count=atoi(argv[1]);
    pthread_t t1,t2;
	pthread_create(&t1, NULL, child,NULL);
	pthread_create(&t2, NULL, parent,NULL);
	pthread_join(t1,NULL);
	pthread_join(t2,NULL);	
    sem_destroy(&mutex);
    sem_destroy(&signal);
    exit(EXIT_SUCCESS);
}