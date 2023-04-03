#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<malloc.h>
#include<string.h>

int main(){
    printf("prog name:\n");
    char prog[256]={0};
    gets(prog);
    printf("arg count:\n");
    char str[256]={0};
    gets(str);
    int argc=atoi(str)+1;
    char** argv=(char**)malloc(sizeof(char[256])*argc);
    argv[0]=prog;
    for(int i=1;i<argc;i++){
        argv[i]=(char*)malloc(256*sizeof(char));
        printf("arg %d:\n",i-1);
        gets(argv[i]);
    }
    execv(prog,argv);
    exit(EXIT_SUCCESS);
}