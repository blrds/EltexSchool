#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

int main(int argc, char* argv[]){
    if(argc!=2)exit(EXIT_FAILURE);
    char str[256]={0};
    FILE* fd=fopen(argv[1],"r");
    fread(str, 1,256, fd);
    fclose(fd);
    printf("%s\n", str);
    exit(EXIT_SUCCESS);
}