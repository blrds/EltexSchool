#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include<stdlib.h>

int main(int argc, char **argv) {
    int sockfd; /* Дескриптор сокета */
    int n, len; /* Переменные для различных длин и количества символов */
    char sendline[1000], recvline[1000]; /* Массивы для отсылаемой и принятой строки */
    struct sockaddr_in servaddr, cliaddr; /* Структуры для адресов сервера и клиента */
    /* Сначала проверяем наличие второго аргумента в
    командной строке. При его отсутствии ругаемся и прекращаем работу */
    if(argc != 3){
        printf("Usage: a.out <IP address port>\n");
        exit(1);
    }
    /* Создаем UDP сокет */
    if((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0){
        perror(NULL); /* Печатаем сообщение об ошибке */
        exit(1);
    }
    /* Заполняем структуру для адреса клиента */
    bzero(&cliaddr, sizeof(cliaddr));
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_port = htons(0);
    cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    /* Настраиваем адрес сокета */
    if(bind(sockfd, (struct sockaddr *) &cliaddr, sizeof(cliaddr)) < 0){
        perror(NULL);
        close(sockfd); /* По окончании работы закрываем дескриптор сокета */
        exit(1);
    }
    /* Заполняем структуру для адреса сервера */
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    if(inet_aton(argv[1], &servaddr.sin_addr) == 0) {
        printf("Invalid IP address\n");
        close(sockfd); /* По окончании работы закрываем дескриптор сокета */
        exit(1);
    }
    /* Вводим строку, которую отошлем серверу */
    while(1){
        printf("=> ");
        fgets(sendline, 1000, stdin);
        if(sendline[strlen(sendline)-1]=='\n')sendline[strlen(sendline)-1]='\0';
        if(sendto(sockfd,sendline,strlen(sendline)+1,0,(struct sockaddr *) &servaddr,sizeof(servaddr)) < 0){
            perror(NULL);
            close(sockfd);
            exit(1);
        }
        if((n = recvfrom(sockfd,recvline,1000,0,(struct sockaddr *) NULL,NULL)) < 0){
            perror(NULL);
            close(sockfd);
            exit(1);
        }
        recvline[n]=0;
        printf("()=>%s\n", recvline);
    }
    close(sockfd);
    return 0;
}