#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h> 
#include <termios.h>
#include <signal.h>

// function declaration for connection establishment
int connection_establishment(int, char**);
int fd;
// function declaration for printing errors and terminating the program
void error(const char *msg)
{
    close(fd);
    printf("%s\n", msg);
    sleep(1);
    exit(0);
}	

void forced_termination_handler(int sig_num){
    signal(SIGINT, forced_termination_handler);
    printf("\nCtrl+C not allowed, please use option-5 to logout\n");
    fflush(stdout);
}

// handles the connection establishment with the server
int connection_establishment(int argc, char *argv[]){
    // checking the format of executing the program
    if(argc != 3){
        error("Proper format not maintained. FORMAT : <executable> <IP address> <Port> .");
        return -1;
    }
    else{
        int port_number;
        int sock_fd;
        struct hostent *server;
        struct in_addr ipv4_address;
        
        // creates the socket based of TCP-IPv4 architecture
        sock_fd = socket(AF_INET, SOCK_STREAM, 0);
        // if there is any error in socket creation, print the error and terminate
        if(sock_fd < 0){
            error("Error in socket creation. Please try again later!!");
        }
        // extract the port number from the argument
        port_number = atoi(argv[2]);
        if(port_number < 0 || port_number > 65535){
            error("Invalid port number.");
        }
        // inet_pton(AF_INET, argv[1], &ipv4_address);
        // server = gethostbyaddr(&ipv4_address, sizeof(ipv4_address), AF_INET);
        // printf("%s\n", server->h_name);
        // locate the host
        server = gethostbyname(argv[1]);
        if(server == NULL){
            error("No such host found!");
        }
        // printf("%s\n", server->h_name);
        // printf("%s\n", inet_ntoa(*(struct in_addr*)server->h_addr));
        // try to connec to the host by specifying the addresses and the correspondingt protocols to be used.
        struct sockaddr_in serv_address;
        memset(&serv_address, 0, sizeof(serv_address));
        serv_address.sin_family = AF_INET;
        memmove((char *)&serv_address.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
        serv_address.sin_port = htons(port_number);
        if(connect(sock_fd, (struct sockaddr*)&serv_address, sizeof(serv_address)) < 0){
            error("Error in connecting, please try again later!!");
        }
        // if connection establishment is successful, then send the socket file descriptor.
        return fd = sock_fd;
    }
}

// it handles login of client to the server
int login(int sock_fd){
    char *id = malloc(100);
    char *password = malloc(100);
    memset(id,0,sizeof(id));
    memset(password,0,sizeof(password));
    // taking input of username and password
    printf("Please enter your ID : ");
    fscanf(stdin, "%s", id);
    // fgets(id, 100, stdin);

    printf("Please enter your password : ");
    // switching the terminal echo OFF so that the password is not visible on the terminal itself
    struct termios old_stat, new_stat;
    if(tcgetattr(STDIN_FILENO, &old_stat)!=0){
        exit(0);
    }
    new_stat = old_stat;
    new_stat.c_lflag &= ~ECHO;
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &new_stat)!=0){
        exit(0);
    }
    // taking the input of password
    fscanf(stdin, "%s", password);
    // resetting the old terminal status. 
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &old_stat);
    char *final_message = malloc(205);
    // sending the message to the server for verification
    snprintf(final_message, 205, "0~%s %s\n", id, password);
    // printf("%s\n", final_message);
    int num_bytes = write(sock_fd, final_message, strlen(final_message));
    if(num_bytes < 0){
        error("Error in writing to socket..");
    }
    // printf("out of it2\n");
    char return_type = '0';
    //reading the reply from server
    num_bytes = read(sock_fd, &return_type, 1);
    // if return type == 0 then the username-passowrd is invalid and the program is closed, else the connection is establisehd for further options 
    if(num_bytes < 0){
        error("Error in reading from socket..");
    }
    if(return_type == '0'){
        close(sock_fd);
        printf("Wrong Credentials!\n");
        return 0;
    }
    else if(return_type == '2')
    {
        close(sock_fd);
    	printf("Already logged in\n");
        return 0;
    }
    printf("\nWelome User. You have 5 options to choose from : (Please abide by the format, without any extra spaces)\n");
    printf("1 : Send Buy Request \n");
    printf("2 : Send Sell Request\n");
    printf("3 : View Order Status\n");
    printf("4 : View Trade Status\n");
    printf("5 : Logout of System \n");
    return 1;
}

int main(int argc, char *argv[])
{
    int sock_fd, n;
    char *message = malloc(1460);
    // initiates the TCp connection and stores the socket file descriptor
    sock_fd = connection_establishment(argc, argv);
    // try to login into the server
    signal(SIGINT, forced_termination_handler);
    if (login(sock_fd) == 0) return 0;
    while(1){
        memset(message,0,sizeof(message));
        int type, num_of_read;
        // entering your choice based on which further actions are performed
        printf("Enter your choice : ");
        num_of_read = scanf("%d%*[^\n]", &type);
        if(num_of_read != 1){
            scanf("%*[^\n]");
            printf("Error in entering input\n");
            continue;
        }
        // handling invalid requests
        if(type <= 0 || type > 5){
            printf("Invalid input\n");
            continue;
        }
        // handling BUY or SELL requests
        if(type == 1 || type == 2){
            int item_code, quantity, unit_price;
            // enter the request type, price, quantity and item code and send it to the server for evaluation
            printf("Enter Item Code, Quantity and Unit Price : ");
            num_of_read = scanf("%d %d %d%*[^\n]", &item_code, &quantity, &unit_price);
            if(num_of_read != 3){
                scanf("%*[^\n]");
                printf("Error in entering input\n");
            	continue;
            }
            if(item_code<1||item_code>10){
                printf("Item code should be from 1 to 10\n");
            	continue;
            }
            snprintf(message, 32, "%d %d %d %d", type, item_code, quantity, unit_price);
            n = write(sock_fd, message, 32);
            char return_type;
            n = read(sock_fd, &return_type, 1);
            if(n < 0){
                error("Error reading from socket");
            }
            // handling invalid requests
            if(return_type == '0'){
                printf("Server discards the input as invalid!");
            }
        }
        else{
            snprintf(message, 32, "%d", type);
            n = write(sock_fd, message, 32);
            // printf("request sent\n");
            // handling the logout request
            if(type == 5){
                char return_type;
                // send the logout request
                n = read(sock_fd, &return_type, 1);
                // printf("5 read");
                if(n < 0){
                    error("Error reading from socket");
                }
                if(return_type == '0'){
                    printf("Server discards the input as invalid!");
                    continue;
                }
                // gracefully closes the connection
                close(sock_fd);
                break;
            }
            else{
                memset(message, 0, sizeof(message));
                // handling the order status
                if(type == 3) {
                    // read the data  
                    n = read(sock_fd, message, 432);
                    if(n < 0){
                        error("Error reading from socket");
                    }
                    else if(n == 0) continue;
                    int i;
                    // print the data onto the screen
                    printf("Item-Code   Best-Buy   Buy-Quantity   Best-Sell   Sell-Quantity\n");
                    char *buffer_scanner = message;
                    int item_code, best_buy, best_sell, buy_quantity, sell_quantity;
                    for(i=1;i<=10;i++){
                        num_of_read = sscanf(buffer_scanner, "%d %d %d %d %d", &item_code, &best_buy, &buy_quantity, &best_sell, &sell_quantity);
                        if(num_of_read != 5){
                            printf("Error in reading input\n");
                            break;
                        }
                        while(buffer_scanner[0] != '\n') buffer_scanner++;
                        buffer_scanner++;
                        printf("%9d %10d %14d %11d %15d\n", item_code, best_buy, buy_quantity, best_sell, sell_quantity);
                    }
                }
                else{
                    // printf("4 is here\n");
                    // read the data  
                    n =  read(sock_fd, message, 1000);
                    // printf("4 is passing\n");
                    if(n < 0){
                        error("Error reading from socket");
                    }
                    else if(n == 0) continue;
                    int i;
                    // print the data onto the screen
                    printf("Item-Code   Type   Person   Quantity      Price\n");
                    char *buffer_scanner = message;
                    int item_code, person, quantity, price;
                    char type;
                    // printf("%s\n", message);
                    while(1){
                        i = sscanf(buffer_scanner, "%d %c %d %d %d", &item_code, &type, &person, &quantity, &price);
                        if(i<=0) break;
                        if(i != 5){
                            printf("Error in reading input\n");
                            break;
                        }
                        while(buffer_scanner[0] != '\n') buffer_scanner++;
                        buffer_scanner++;
                        printf("%9d %6c %8d %10d %10d\n", item_code, type, person, quantity, price);
                    }
                }
            }
        }
    }
    return 0;
}