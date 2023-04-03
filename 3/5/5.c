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
    char tmp[256]={0};
    gets(str);
    int argc=atoi(str)+1;
    char** argv=(char**)malloc(sizeof(char[256])*argc);
    argv[0]=prog;
    strcpy(str,"");
    for(int i=1;i<argc;i++){
        argv[i]=(char*)malloc(256*sizeof(char));
        printf("arg %d:\n",i-1);
        gets(argv[i]);
        strcpy(tmp,str);
        sprintf(str, "%s %s",tmp,argv[i]);
    }
    strcpy(tmp,str);
    sprintf(str,"%s %s &",prog,tmp);
    system(str);
    exit(EXIT_SUCCESS);
}