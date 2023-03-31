#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

int main(int argc, char *argv[]){
    int i=1,j=argc;
    __pid_t p=fork();
    if(p==0)i=(int)argc/2+1;
    else j=(int)argc/2+1;
    for(;i<j;i++){
        int a=atoi(argv[i]);
        printf("%d ", a*a);
    }
    exit(EXIT_SUCCESS);
}