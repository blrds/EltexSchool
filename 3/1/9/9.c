#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>
#include<string.h>

void parent(int readFrom, int writeTo){
    FILE *read=fdopen(readFrom, "r");
    FILE *write=fdopen(writeTo, "w");
    int c;
    char str[256]={0}, toInt[256]={0};
    int i=0;//int из-за fgetc(EOF=-1)
    int j=0;
    while(1){
        c=fgetc(read);//будет ждать нового символа
        str[i]=(char)c;
        i++;
        if(c=='\0')break;//спец символ, чтобы понять что пайп все
        if(c==' '){
            toInt[j]='\0';
            int a=atoi(toInt);
            fprintf(write, "%d ", (a*2));
            j=0;
        }
        else{
            toInt[j]=(char)c;
            j++;
        }
    }
    fprintf(write,"%c",'\0');
    fclose(read);
    fclose(write);
    printf("parent got:%s\n", str);
    FILE *f=fopen("parent.txt", "w");
    fwrite(&str,sizeof(char),strlen(str),f);
    fclose(f);
}

void child(int count, int readFrom, int writeTo){
    srand(time(NULL));
    FILE *write=fdopen(writeTo, "w");
    int j=0;
    for(int i=0;i<count;i++){
        j=rand()%20;
        fprintf(write, "%d ", j);
    }
    fprintf(write,"%c", '\0');
    fclose(write);

    FILE *read=fdopen(readFrom, "r");
    char str[256]={0};
    int c=0;
    int i=0;
    while(1){
        c=fgetc(read);//будет ждать нового символа
        str[i]=(char)c;
        i++;
        if(c=='\0')break;//спец символ, чтобы понять что пайп все
    }
    fclose(read);
    printf("child got:%s\n", str);
    FILE *f=fopen("child.txt", "w");
    fwrite(&str,sizeof(char),strlen(str),f);
    fclose(f);
}

int main(int argc, char* argv[]){
    if(argc!=2)exit(EXIT_FAILURE);
    __pid_t pid;
    int pipe1[2], pipe2[2];
    pipe(pipe1);
    pipe(pipe2);
    pid=fork();
    if(pid==0)child(atoi(argv[1]), pipe2[0],pipe1[1]);
    else parent(pipe1[0],pipe2[1]);
    exit(EXIT_SUCCESS);
}