/* A simple server in the internet domain using TCP
   The port number is passed as an argument 
   This version runs forever, forking off a separate 
   process for each connection
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void dostuff(int); /* function prototype */
int login(char*);
void buy(char*);
void sell(char*);
void order_status(char*);
void trade_status(char*);
void error(const char *msg){
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]){
    int sockfd, newsockfd, portno, pid;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    if (argc < 2)    {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        error("ERROR opening socket");
    }
    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        error("ERROR on binding");
    }
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    while (1){
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0){
            error("ERROR on accept");
        }
        dostuff(newsockfd);
        // close(newsockfd);
    } /* end of while */
    close(sockfd);
    return 0; /* we never get here */
}

/******** DOSTUFF() *********************
 There is a separate instance of this function 
 for each connection.  It handles all communication
 once a connnection has been established.
 *****************************************/
void dostuff(int sock)
{
    int n;
    char buffer[256];

    bzero(buffer, 256);
    n = read(sock, buffer, 255);
    if (n < 0){
        error("ERROR reading from socket");
    }
    printf("Here is the message: %s\n", buffer);
    char request_type;
    int flag=0;
    request_type=buffer[0];
    if(request_type == '1'){
        flag=login(buffer);
        if(flag==0){
            close(sock);
            return;
        }
        else{
            dostuff(sock);
        }
    }
    else if(request_type == '2'){
        // buy(buffer);
    }
    else if(request_type == '3'){
        // sell(buffer);
    }
    else if(request_type == '4'){
        // order_status(buffer);
    }
    else{
        // trade_status(buffer);
    }
    n = write(sock, "I got your message", 18);
    if (n < 0)
        error("ERROR writing to socket");
}

int login(char* buffer){
    char trader_id=buffer[2];
    char pass[32];
    int pass_len = 0;
    for(int i=4;buffer[i] != '\0';i++){
        pass[i-4]=buffer[i];
        pass_len++;
    }
    pass[pass_len] = '\0';
    FILE* fp;
    fp = fopen("login.txt", "r");
    if (fp == NULL){
        perror("Error while opening the file.\n");
        exit(EXIT_FAILURE);
    }
    char ch;
    char passf[32];
    while((ch = fgetc(fp)) != EOF){
        char trader_idf = ch;
        ch = fgetc(fp);  // reads space
        ch = fgetc(fp);  // reads first pass character
        for(int i=0;ch!='\n'&&ch!=EOF;i++){
            passf[i] = fgetc(fp);
            passf[i+1] = '\0';
            ch = fgetc(fp);
        }
        if(trader_id == trader_idf){
            if(strncmp(pass, passf, pass_len)==0){
                return 1;
            }
        }
    }
    return 0;
}