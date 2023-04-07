#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

int main(int argc, char* argv[]){
    srand(time(NULL));
    char fifoFile[]="/tmp/fifo0001.1";
    int fifo;
    char str[80]={0};
    if(argc==2 && atoi(argv[1])==-1)unlink(fifoFile);
    if((mkfifo(fifoFile,O_RDWR))==-1){
        fprintf(stderr,"cant create fifo\n");
        exit(EXIT_FAILURE);
    }
    while(1){
        sleep(3);
        if((fifo=open(fifoFile,O_WRONLY))==-1){
            fprintf(stderr,"cant open fifo\n");
            exit(EXIT_FAILURE);
        }
           sprintf(str,"%d\0",(rand()%20));
           write(fifo,str,strlen(str));
           printf("%s\n", str);
    
        close(fifo);
    }
    exit(EXIT_SUCCESS);
}
