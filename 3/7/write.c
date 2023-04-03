#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<string.h>

int main(int argc, char* argv[]){
    if(argc!=2)exit(EXIT_FAILURE);
    char str[256]={0};
    gets(str);
    FILE* fd=fopen(argv[1], "w");
    fwrite(&str, sizeof(char), strlen(str), fd);
    fclose(fd);
    exit(EXIT_SUCCESS);
}