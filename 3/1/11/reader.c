#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

int main(){
    int fifo;
    char fifoFile[]="/tmp/fifo0001.1";
    char str[80]={0};
    while(1){
        if((fifo=open(fifoFile,O_RDWR))==-1){
            fprintf(stderr,"cant open fifo\n");
            exit(EXIT_FAILURE);
        }
        sprintf(str,"\0");
        if((read(fifo,&str, sizeof(str)))==-1){
            fprintf(stderr,"cant read fifo\n");
            exit(EXIT_FAILURE);
        }else
            printf("%s\n",str);
        close(fifo);
    }
    exit(EXIT_SUCCESS);
}
