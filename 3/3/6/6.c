#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>
#include<string.h>
#include<semaphore.h>
#include<pthread.h>
#include<fcntl.h>

#include<sys/types.h>
#include<sys/stat.h>

sem_t sem_mutex, sem_signal, sem_reader_count;
int _pipe[2], count;

void* parent(void* arg){
    int pipe=_pipe[0];
    char str[256]={0};
    for(int i=0;i<count*2;i++){
        sleep(1);
        printf("parent %d\n",i);
        int c;
        memset(str, '\0', sizeof(str));
        read(pipe, str,sizeof(str));
        printf("parent red %s\n", str);
        sem_wait(&sem_mutex);
        FILE *f=fopen("a.txt", "w");
        fwrite(&str,sizeof(char),strlen(str),f);
        fclose(f);
        printf("parent wrote\n");
        sem_post(&sem_mutex);
        sem_post(&sem_signal);
    }
    
}


void* child(void* arg){
    srand(time(NULL));
    int pipe=_pipe[1];
    int j=0;
    char str[256]={0};
    int id=gettid();
    sem_wait(&sem_reader_count);
    for(int i=0;i<count;i++){
        sleep(1);
        printf("child%d %d\n",id,i);
        j=rand()%20;
        memset(str,'\0',sizeof(str));
        sprintf(str,"%d%c",j,'\0');
        write(pipe, str, strlen(str));
        printf("child%d wrote to pipe %d\n",id,j);
        sem_wait(&sem_signal);
        sem_wait(&sem_mutex);
        FILE *f=fopen("a.txt", "r");
        char str[256]={0};
        fgets(str,256,f);
        fclose(f);
        printf("Child%d red: %s\n",id,str);
        sem_post(&sem_mutex);
    }
    sem_post(&sem_reader_count);
}

int main(int argc, char* argv[]){
    sem_init(&sem_mutex,0,1);
    sem_init(&sem_signal,0,0);
    sem_init(&sem_reader_count,0,1);
    if(argc!=2)exit(EXIT_FAILURE);
    count=atoi(argv[1]);
    pipe(_pipe);

    pthread_t p,c1,c2;
    pthread_create(&c1,NULL,child,NULL);
    sleep(1);
    pthread_create(&c2,NULL,child,NULL);
    pthread_create(&p,NULL,parent,NULL);
    pthread_join(p,NULL);
    pthread_join(c1,NULL);
    pthread_join(c2,NULL);
    close(_pipe[0]);
    close(_pipe[1]);
    sem_destroy(&sem_mutex);
    sem_destroy(&sem_signal);
    sem_destroy(&sem_reader_count);
    exit(EXIT_SUCCESS);
}