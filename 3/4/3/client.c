#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int my_sock, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    //char buff[256];
	char buff[1024];
    printf("TCP DEMO CLIENT\n");
	
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    // извлечение порта
	portno = atoi(argv[2]);
    
	// Шаг 1 - создание сокета
	my_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (my_sock < 0) 
        error("ERROR opening socket");
    // извлечение хоста
	server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    // заполенние структуры serv_addr
	bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
        (char *)&serv_addr.sin_addr.s_addr,
        server->h_length);
    // установка порта
	serv_addr.sin_port = htons(portno);
    
	// Шаг 2 - установка соединения	
	if (connect(my_sock,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    #define operations 4
	// Шаг 3 - чтение и передача сообщений
    for(int i=0;i<operations;i++){
	    n = recv(my_sock, &buff[0], sizeof(buff) - 1, 0);
        // ставим завершающий ноль в конце строки
        buff[n] = 0;
        if (strcmp(buff, "quit\0")==0){
            // Корректный выход
            printf("Exit...1");
            close(my_sock);
             return 0;
        }

        // выводим на экран
        printf("S=>C:%s\n", buff);
        memset(buff, 0,sizeof(buff));
        // читаем пользовательский ввод с клавиатуры
    	if(operations!=i+1){
            printf("S<=C:"); 
            fgets(&buff[0], sizeof(buff) - 1, stdin);
            if(buff[strlen(buff)-1]=='\n')buff[strlen(buff)-1]='\0';
            send(my_sock, &buff[0], strlen(&buff[0]), 0);
        }
    }
    close(my_sock);
    return EXIT_SUCCESS;
}
