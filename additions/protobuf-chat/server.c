#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<time.h>

#define MAX_NAME_LENGTH 25
#define MAX_MESSAGE_LENGTH 4096

typedef struct Client{
    char name[MAX_NAME_LENGTH];
    int socket;
}Client;

typedef struct Message{
    char message[4096];
    Client author;
    time_t born_time;
}Message;

void INTListener(int signal){
    exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]){
    signal(SIGINT,INTListener);

    return EXIT_SUCCESS;
}