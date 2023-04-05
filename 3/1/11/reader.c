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
    char str[80];
    while(1){
        fgets(str, 80, stdin);
        if((fifo=open(fifoFile,O_RDONLY))==-1){
            fprintf(stderr,"cant create fifo\n");
            exit(EXIT_FAILURE);
        }
        if((read(fifo,&str, strlen(str)))==-1){
            fprintf(stderr,"cant read fifo\n");
            exit(EXIT_FAILURE);
        }else
            printf("A%sA\n",str);
        close(fifo);
    }
    exit(EXIT_SUCCESS);
}