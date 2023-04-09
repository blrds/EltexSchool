#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

char** strSplit(char* string, char* delim, int* size){
    char *token=strtok(string, delim);
    int i=0;
    char** array=(char**)malloc(sizeof(char*));
    array[0]=(char*)malloc(sizeof(char)*strlen(token));
    while(token){
        array=(char**)realloc(array,sizeof(char*)*(i+1));
        array[i]=(char*)malloc(sizeof(char)*strlen(token));
        strcpy(array[i],token);
        token=strtok(NULL,delim);
        i++;
    }
    (*size)=i;
    return array;
}

int main(int argc, char* argv[]){
    int status;
    char h[]="ls| head -3 | tail -1";
    char *path="/bin/";
    int i=0,length=0,j;
    char** tokens=strSplit(h,"|",&length);
    char* token, *arg;
    char str[256]={0};
    for(i=0;i<length;i++){
        int _pipe[2]; 
        pipe(_pipe);
        if ( fork()== 0 ){
            char** args=strSplit(tokens[i]," ",&j);
            char tmp[256]={0};
            fprintf(stderr,"child: iteration %d\n",i);
            strcpy(tmp,args[0]);
            sprintf(args[0],"%s%s",path,tmp);
            if(args[1]!=NULL){
                args=(char**)realloc(args,sizeof(char*)*3);
                args[2]="out.txt";
            }
            fprintf(stderr,"child: %s %s %s\n", args[0],args[1], args[2]);
            close(_pipe[0]);
            dup2(_pipe[1], 1);
            execv( args[0], args );
            exit(EXIT_SUCCESS);
        }else{
            close(_pipe[1]);
            wait( &status );
            printf("parent: child status=%d\n",status);
            FILE *file=fopen("out.txt","w+");
            sprintf(str,"%s","\0");
            read(_pipe[0],&str, sizeof(str));
            printf("parent: %s %s\n", tokens[i], str);
            fprintf(file,"%s",str);
            fclose(file);
        }
    }
    printf("A%sA\n", str);
    exit(EXIT_SUCCESS);
}