#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

int nclients = 0;
pid_t ppid;

void USR1Listener(int signal){
    if(signal!=SIGUSR1)return;
    nclients--;
}

void error(const char *msg){
    perror(msg);
    exit(1);
}

void printusers(){ 
	if(nclients)printf("%d user on-line\n",nclients);
	else printf("No User on line\n");
}

void dostuff (int sock){
	int bytes_recv;
	int a,b;
	char buff[1025];
    FILE* fp=fopen("original.txt","rb");
   	while(fgets(buff,1024,fp)!=NULL){
        buff[1024]='\0';
        write(sock,&buff[0],sizeof(buff));
        sleep(1);
        memset(buff,'\0',sizeof(buff));
    }
    fclose(fp);
    snprintf(buff, sizeof(buff), "-1\0");
    write(sock,buff,sizeof(buff));
    printf("-disconnect\n"); 
    kill(ppid,SIGUSR1);
    nclients--;
	printusers();
	return;
}


int main(int argc, char *argv[]){
    signal(SIGUSR1, USR1Listener);
    char buff[1024];
	int sockfd, newsockfd;
	int portno;
	int pid;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    if (argc < 2){
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
       error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
	listen(sockfd,5);
    clilen = sizeof(cli_addr);   
	while (1){
        newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) 
            error("ERROR on accept");
	    nclients++;
		/*
		struct hostent *hst;
		hst = gethostbyaddr((char *)&cli_addr.sin_addr, 4, AF_INET);
		printf("+%s [%s] new connect!\n",
		(hst) ? hst->h_name : "Unknown host",
		(char*)inet_ntoa(cli_addr.sin_addr));*/
	    printusers();
        pid = fork();
        if (pid < 0)
            error("ERROR on fork");
        if (pid == 0){
            close(sockfd);
            dostuff(newsockfd);
            exit(0);
        }
        else {
            close(newsockfd);
            wait(NULL);
        }
    } 
    close(sockfd);
    return 0; 
}