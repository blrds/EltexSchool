#include<errno.h>
#include<netdb.h>
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

void INTListener(int signal){
    
    exit(EXIT_SUCCESS);
}

int pflag=0;

void TSTPListener(int signal){
    pflag=1;
}

int main(int argc, char* argv[]){//argv[1] - address | argv[2] - port | argv[3] - name
    signal(SIGINT,INTListener);
    signal(SIGTSTP,TSTPListener);
    int master_socket, valread;
    int activity, addrlen;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    fd_set clients_fds;

    char buffer[MAX_BUFF_LENGTH]="";

    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0){  
        perror("socket failed");  
        exit(EXIT_FAILURE);  
    }

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
	bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
        (char *)&serv_addr.sin_addr.s_addr,
        server->h_length);
        
	serv_addr.sin_port = htons(atoi(argv[2]));
    if (connect(master_socket,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    Message msg = MESSAGE__INIT;
    msg.author=malloc(MAX_NAME_LENGTH);
    memcpy(msg.author,argv[4], strlen(argv[4]));
    msg.text=malloc(MAX_MESSAGE_LENGTH);
    sprintf(msg.text, "/reg");
    message__pack(&msg,buffer);
    send(master_socket, buffer, strlen(buffer), 0);
    while(1){
        FD_ZERO(&clients_fds);
        FD_SET(master_socket, &clients_fds);  
        activity=select(master_socket+1, &clients_fds, NULL, NULL, NULL);
        if ((activity < 0) && (errno!=EINTR)){  
            printf("select error");  
        }  
        if (FD_ISSET(master_socket, &clients_fds)){  
            if ((valread = read(master_socket, buffer, MAX_BUFF_LENGTH)) == 0){  
                    Message *m=message__unpack(NULL, valread, buffer);
                    printf("%s:%s", m->author, m->text);
                } 
        }
        if(pflag){
            pflag=0;
            scanf("%s", buffer);
            send(master_socket, buffer, strlen(buffer), 0);
        }
        
    }
    return EXIT_SUCCESS;
}