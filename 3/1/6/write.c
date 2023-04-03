#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

int main(int argc, char* argv[]){
    if(argc!=2)exit(EXIT_FAILURE);
    char str[256]={0};
    gets(str);
    creat(argv[1],S_IRWXU);
    int fd=open(argv[1],O_WRONLY);
    write(fd,str,256);
    close(fd);
    exit(EXIT_SUCCESS);
}