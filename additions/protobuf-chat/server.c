#include<errno.h>
#include<signal.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<unistd.h>

#include<arpa/inet.h>
 
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#include <netinet/in.h>

#include "Message.pb-c.h"

#define MAX_NAME_LENGTH 25
#define MAX_MESSAGE_LENGTH 4096
#define PORT 5555
#define MAX_CLIENTS 10
#define MAX_BUFF_LENGTH MAX_MESSAGE_LENGTH+MAX_NAME_LENGTH

typedef struct Client{
    char name[MAX_NAME_LENGTH];
    int socket;
}Client;

Client Clients[MAX_CLIENTS];

void INTListener(int signal){
    for(int i=0;i<MAX_CLIENTS;i++){
        close(Clients[i].socket);
        free(Clients[i].name);
    }
    exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]){
    signal(SIGINT,INTListener);
    int master_socket, new_socket, valread;
    int sd, max_sd, activity, addrlen;
    struct sockaddr_in address;  
    fd_set clients_fds;
    char buffer[MAX_BUFF_LENGTH]="";

    char *greeting_message = "You've been connected to server\n\r";
    char *farewell_message = "You've been disconnected from server\n\r";

    for (int i = 0; i < MAX_CLIENTS; i++){  
        Clients[i].socket = 0;  
    } 
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0){  
        perror("socket failed");  
        exit(EXIT_FAILURE);  
    }

    address.sin_family = AF_INET;  
    address.sin_addr.s_addr = INADDR_ANY;  
    address.sin_port = htons(PORT); 
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0){  
        perror("bind failed");  
        exit(EXIT_FAILURE);  
    } 
    if (listen(master_socket, 3) < 0)  
    {  
        perror("listen");  
        exit(EXIT_FAILURE);  
    } 

    while(1){
        FD_ZERO(&clients_fds);
        FD_SET(master_socket, &clients_fds);  
        max_sd = master_socket;  
             
        for (int i = 0 ; i < MAX_CLIENTS ; i++){  
            sd = Clients[i].socket;  
            if(sd > 0)  
                FD_SET( sd , &clients_fds);  
            if(sd > max_sd)  
                max_sd = sd;  
        }

        activity=select(max_sd+1, &clients_fds, NULL, NULL, NULL);
        if ((activity < 0) && (errno!=EINTR)){  
            printf("select error");  
        }  
             
        if (FD_ISSET(master_socket, &clients_fds)){  
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0){  
                perror("accept");  
                exit(EXIT_FAILURE);  
            }  
            
            /*if(send(new_socket, greeting_message, strlen(greeting_message), 0) != strlen(greeting_message)){  
                perror("send");  
            }*/
                 
            for (int i = 0; i < MAX_CLIENTS; i++){  
                if(Clients[i].socket == 0 ){  
                    Clients[i].socket = new_socket;
                    break;  
                }  
            } 
        } 

        for (int i = 0; i < MAX_CLIENTS; i++){  
            sd = Clients[i].socket;  
                 
            if (FD_ISSET(sd, &clients_fds)){
                if ((valread = read(sd, buffer, MAX_BUFF_LENGTH)) == 0){  
                    Message *msg=message__unpack(NULL, valread, buffer);
                    getpeername(sd, (struct sockaddr*)&address,(socklen_t*)&addrlen);  
                    if(strcmp(msg->text,"/reg")==0){// /reg
                        memcpy(Clients[i].name,msg->author, strlen(msg->author));
                        send(Clients[i].socket, greeting_message, strlen(greeting_message),0);
                        continue;
                    }
                    if(strcmp(msg->text,"/exit")==0){// /close
                        //send(Clients[i].socket, farewell_message, strlen(farewell_message),0);
                        close(sd);  
                        Clients[i].socket = 0;
                        memset(Clients[i].name,0,MAX_NAME_LENGTH);
                        continue;
                    }
                    message__pack(msg,buffer);
                    for(int j=0;j<MAX_CLIENTS;j++){
                        if(i!=j){
                            send(Clients[j].socket, buffer, strlen(buffer), 0);
                        }
                    }
                }
            }  
        } 
    }
    return EXIT_SUCCESS;
}