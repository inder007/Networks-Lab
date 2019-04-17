#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <arpa/inet.h>
#include "linked_list.c"

typedef struct connections
{
	int sockfd; // attaches file descriptor for each connection.
	int login_id; //tells us to which user this fd is related.
	struct sockaddr_in address; 
} connections;

extern int ll_id;
int open_ports;
struct list* buy_list[11]; // array of lists, one for each item who is ready to buy.
struct list* sell_list[11]; // array of lists, one for each item who is ready to sell.
struct list* matched[6]; // lists of matched traes for each trader
connections conn_soc[5];	//as given that there are maximum 5 clients only, therefore made array of 5 for same of struct connections
int sockfd;
// max function
int max(int a, int b)
{
	if (a > b)
		return a;
	return b;
}

// error function
void error(const char *msg)
{
	printf("%s\n", msg);
	exit(1);
}

// This function is called everytime a buy request comes to server to see if any suitable trade is available
void match_buy(int t_id, int i_id, int price, int quant){
	// searches for minimum trade in sell_list
    node* mini = min_search(sell_list[i_id]);
    if(mini != NULL && mini->price <= price){
        if(mini->quant >= quant){
			// When there is a sell trade that sufficed current buy request partially or fully
            //seller matched
            insert(matched[mini->t_id],quant,t_id,mini->i_id,mini->price,'s');
            mini->quant-=quant;
            //buyer matched
            insert(matched[t_id],quant,mini->t_id,mini->i_id,mini->price,'b');
            if(mini->quant == 0){
                delete(sell_list[i_id],mini);
            }
        }
        else{
			// When there is a sell trade that is not sufficed fully by current buy trade
            //seller matched
            insert(matched[mini->t_id],mini->quant,t_id,mini->i_id,mini->price,'s');
            quant-=mini->quant;
            //buyer matched
            insert(matched[t_id],mini->quant,mini->t_id,mini->i_id,mini->price,'b');
            delete(sell_list[i_id],mini);
            match_buy(t_id,i_id,price,quant);
        }
    }
    else{
		// When there is no matching trade
        insert(buy_list[i_id],quant,t_id,i_id,price,'b');
    }
}

// This function is called everytime a sell request comes to server to see if any suitable trade is available
void match_sell(int t_id, int i_id, int price, int quant){
    // searches for minimum trade in buy_list
	node* maxi = max_search(buy_list[i_id]);
    if(maxi != NULL && maxi->price>=price){
        if(maxi->quant >= quant){
			// When there is a buy trade that sufficed current buy request partially or fully
            //seller matched
            insert(matched[maxi->t_id],quant,t_id,maxi->i_id,maxi->price,'b');
            maxi->quant-=quant;
            //buyer matched
            insert(matched[t_id],quant,maxi->t_id,maxi->i_id,maxi->price,'s');
            if(maxi->quant == 0){
                delete(buy_list[i_id],maxi);
            }
        }
        else{
			// When there is a buy trade that is not sufficed fully by current sell trade
            //seller matched
            insert(matched[maxi->t_id],maxi->quant,t_id,maxi->i_id,maxi->price,'b');
            quant-=maxi->quant;
            //buyer matched
            insert(matched[t_id],maxi->quant,maxi->t_id,maxi->i_id,maxi->price,'s');
            delete(buy_list[i_id],maxi);
            match_sell(t_id,i_id,price,quant);
        }
    }
    else{
		// When there is no matching trade
        insert(sell_list[i_id],quant,t_id,i_id,price,'s');
    }
}

//This function builds the set of file descripter which have a established connection
int build_set(fd_set *read_set, int listensock, connections *conn_soc)
{
	int i;
	int maxm = 0;
	FD_ZERO(read_set);
	// Adding welcome socket to fd_set
	FD_SET(listensock, read_set);
	maxm = listensock;
	open_ports=0;
	for (i = 0; i < 5; i++)
	{
		// Adding the trader socket if connection is established
		if (conn_soc[i].sockfd != -1)
		{
			open_ports++;
			// printf("this %d is open\n", i);
			FD_SET(conn_soc[i].sockfd, read_set);
			// max fd number is reqired by select statement
			maxm = max(maxm, conn_soc[i].sockfd);
		}
	}
	// printf("Open ports : %d\n", open_ports);
	return maxm;
}

// Called to close server properly by closing welcoming socket
void shutdown_properly(int code, int listen_sock, connections *con)
{
	int i;
	// Closes all other five traders
	for (i = 0; i < 5; ++i)
		if (con[i].sockfd != -1)
			close(con[i].sockfd);
	close(listen_sock);
	sleep(1);
	printf("Shutdown server properly.\n");
	exit(code);
}

// Handling new incoming connection from a trader
int handle_new_connection(int sock_fd, connections *conn)
{
	struct sockaddr_in client_addr;
	memset(&client_addr, 0, sizeof(client_addr));
	socklen_t client_len = sizeof(client_addr);
	//Accept the new incoming connection from a trader
	int new_client_sock = accept(sock_fd, (struct sockaddr *)&client_addr, &client_len);
	if (new_client_sock < 0)
	{
		// error in establishing connection with the trader
		perror("accept()");	
		return -1;
	}
	
	char client_ipv4_str[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &client_addr.sin_addr, client_ipv4_str, INET_ADDRSTRLEN);

	printf("Incoming connection from %s:%d.\n", client_ipv4_str, client_addr.sin_port);

	int i;
	for (i = 0; i < 5; ++i)
	{
		// getting  a free socket for connection establishment
		if (conn[i].sockfd == -1)
		{
			conn[i].sockfd = new_client_sock;
			conn[i].address = client_addr;
			return 0;
		}
	}

	// If there is attempt to connect with more than 5 connections
	printf("There is too much connections. Close new connection %s:%d.\n", client_ipv4_str, client_addr.sin_port);
	close(new_client_sock);
	return -1;
}

// Return 1 if username and password match, otherwise 0
int login(char *buffer, int n){
	int type;
	type = buffer[0]-'0';
	buffer++;
	// Excludes the delimiter from messages
	if(buffer[0] == '~') buffer++;
	else return 0;

	// Opening login files
	FILE* fp;
	fp = fopen("login.txt", "r");
	if(fp == NULL){
		return 0;
	}
	char nsp[254];
	memset(nsp, 0, sizeof(nsp));
	while(fgets(nsp, 254, fp)!=NULL){
		// printf("%d %d\n", (int)strlen(buffer), (int)strlen(nsp));
		//compares username and password together
		if(strcmp(buffer, nsp) == 0) {
			return 1;
		}
		// sets nsp to null characters
		memset(nsp, 0, sizeof(nsp));
	}
	return 0;
}

// used to handle Ctrl+C
void forced_termination_handler(int sig_num){
	char status;
	// printf("Do you want to exit? ('y' for yes) : ");
	// scanf("%c", &status);
	// if(status == 'y')
	shutdown_properly(0,sockfd,conn_soc);
    // signal(SIGINT, forced_termination_handler);
    fflush(stdout);
}

int main(int argc, char *argv[])
{
	//The fd_set data type represents file descriptor sets for the select function. It is actually a bit array.
	char clear_database;
	printf("Do you want to clear database?(y|n)");
	scanf("%c", &clear_database);
	if(clear_database=='y'){
		clear_files();
	}

	ll_id = 1;
	fd_set read_set;
	
	int i = 0,j;
	// intialised all file descriptors to -1 representing that it doesn't currently point to anything.
	for (i = 0; i < 5; i++)
	{
		conn_soc[i].sockfd = -1;
	}
	//intialised buy and sell lists.
	for(i=1;i<11;i++) {
		buy_list[i] = create_list();
		sell_list[i] = create_list();	
	}
	//intialised matched list.
	for(i=1;i<6;i++){
		matched[i] = create_list();
	}
	signal(SIGINT, forced_termination_handler);
	int newsockfd, portno;
	socklen_t clilen;
	char buffer[2000];
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	if (argc < 2)
	{
		fprintf(stderr, "ERROR, no port provided\n");
		exit(1);
	}
	// socket creation in C
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0){
		error("ERROR opening socket");
	}
	bzero((char *)&serv_addr, sizeof(serv_addr)); //put all zero values in serv_addr
	portno = atoi(argv[1]); //converts port no to integer
	//defining all values of serv_addr
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0){ //bind assigns the details specified in the structure ‘serv_addr’ to the socket
		error("ERROR on binding");
	}
	// listen tells maximum number of client connections that server will queue for this listening socket, which here is 5
	listen(sockfd, 5);

	clilen = sizeof(cli_addr);
	while (1)
	{
		//read set is populated here	
		int maxm = build_set(&read_set, sockfd, conn_soc);
		//The select function blocks the calling process until there is activity on any of the specified sets of file descriptors.
		int activity = select(maxm + 1, &read_set, NULL, NULL, NULL);
		//based on return value of select different cases arises. if activity>0 it means there was no error.
		switch (activity)
		{
			case -1:
				perror("select() failed");
				shutdown_properly(0, sockfd, conn_soc);
				break;
			case 0:
				perror("select() failed");
				shutdown_properly(0, sockfd, conn_soc);
				break;
			default:
				//FD_ISSET() returns a nonzero value (true) if sockfd is a member of the file descriptor set read_set, and zero (false) otherwise.
				//now if request has come to sockfd and not to some other file desc., it means it is a new connection which is to be handled.
				if (FD_ISSET(sockfd, &read_set)){
					//handles new connection.
					handle_new_connection(sockfd, conn_soc);
				}
				//runs if a request has come to already intialised port
				else{
					//looping to check on which of the 5 sockets the request has arrive.
					for (i = 0; i < 5; i++){
						if (conn_soc[i].sockfd != -1 && FD_ISSET(conn_soc[i].sockfd, &read_set)){
							bzero(buffer, 256);
							//reads the message from the buffer.
							n = read(conn_soc[i].sockfd, buffer,32);
							if (n < 0){
								error("ERROR reading from socket");
							}
							else if(n == 0){
								break;
							}
							printf("Here is the message: %s\n", buffer);
							int item_code, quantity, unit_price, type;
							//packet format: first alphabet is always what is request type from client.
							switch (buffer[0]){
								// 0 means it is login request.
								//hence it will have username and password as rest of message.
								case '0':
									//checks if login is successfull, if it is then send ok, i.e., 1 back else send 0 back.
									if(login(buffer, n)){
									
										int person_id;
										person_id = buffer[2]-'0';
										int found=0,j;
										for(j=0;j<5;j++){
											if(conn_soc[j].sockfd != -1 && conn_soc[j].login_id == person_id){
												found=1;
												break;
											}
										}
										if(found){
											n = write(conn_soc[i].sockfd, "2", 1);
											close(conn_soc[i].sockfd);
											conn_soc[i].sockfd = -1;
											conn_soc[i].login_id = -100 ;
											printf("already logged in\n");
										}
										else{
											n = write(conn_soc[i].sockfd, "1", 1);
											conn_soc[i].login_id = person_id;
											printf("login successful\n");
										}
										
									}
									else{
										n = write(conn_soc[i].sockfd, "0", 1);
										close(conn_soc[i].sockfd);
										conn_soc[i].sockfd = -1;
										conn_soc[i].login_id = -100 ;
										printf("login unsuccessful\n");
									}
									if (n < 0){
										error("ERROR writing to socket");
										close(conn_soc[i].sockfd);
										conn_soc[i].sockfd = -1;
										conn_soc[i].login_id = -100 ;
									}
									
									break;
								// 1 means we have buy request
								case '1' :
									//scans buffer and separates all required entities into different vaariables.
									sscanf(buffer, "%d %d %d %d", &type, &item_code, &quantity, &unit_price);
									printf("%d %d %d %d %d\n", conn_soc[i].login_id, type, item_code, quantity, unit_price);
									// calls match buy function to check if there can be a match.
									match_buy(conn_soc[i].login_id, item_code, unit_price, quantity);
									n = write(conn_soc[i].sockfd, "1", 1);
									break; 
								// 2 means we have sell request from the trader.
								case '2' :
									sscanf(buffer, "%d %d %d %d", &type, &item_code, &quantity, &unit_price);
									printf("%d %d %d %d %d\n", conn_soc[i].login_id, type, item_code, quantity, unit_price);
									// calls match sell function to check if there can be a match.
									match_sell(conn_soc[i].login_id, item_code, unit_price, quantity);
									n = write(conn_soc[i].sockfd, "1", 1);
									break;
								// 3 means we have reply to trader, order status in which we tells him for each item best buy and sell prices.
								case '3' :
									memset(buffer, 0, sizeof(buffer));
									char temp_string[43];
									struct node *pointer_buy, *pointer_sell;
									int best_buy, best_sell, buy_quantity, sell_quantity;
									//iterate over each item to send to trader best buy and sell prices
									for(j=1;j<=10;j++){
										memset(temp_string, 0, sizeof(temp_string));
										best_buy = best_sell = buy_quantity = sell_quantity = 0;
										//find if any with max price to buy and min price to sell and store there values.
										pointer_buy = max_search(buy_list[j]);
										pointer_sell = min_search(sell_list[j]);
										if(pointer_buy != NULL){
											best_buy = pointer_buy->price;
											buy_quantity = pointer_buy->quant;
										}
										if(pointer_sell != NULL){
											best_sell = pointer_sell->price;
											sell_quantity = pointer_sell->quant;
										}
										//copy all required values in temp_string.
										sprintf(temp_string, "%d %d %d %d %d\n",j, best_buy, buy_quantity, best_sell, sell_quantity);
										//concatenate it for all items.
										strcat(buffer, temp_string);
									}
									//send the required message back to trader.
									n = write(conn_soc[i].sockfd, buffer, 432);
									if (n < 0){
										error("ERROR writing to socket");
										close(conn_soc[i].sockfd);
										conn_soc[i].sockfd = -1;
										conn_soc[i].login_id = -100 ;
									}
									printf("3 done\n");
									break; 
								// 4 means to reply trader with his trade status
								case '4' :
									memset(buffer, 0, sizeof(buffer));
									struct node* head = matched[conn_soc[i].login_id]->head;
									memset(temp_string, 0, 43);
									char type;
									int quantity, price, login_id;
									// if nothing is there in matched list,then send 0's.
									if(head == NULL){
										sprintf(buffer, "0 x 0 0 0\n");
									}
									// else find all matched orders, concatenate all of them in one and then send them back to trader.
									while(head != NULL){
										memset(temp_string, 0, 43);
										sprintf(temp_string, "%d %c %d %d %d\n", head->i_id, head->bors, head->t_id, head->quant, head->price);
										head = head->next;
										strcat(buffer, temp_string);
									}
									n = write(conn_soc[i].sockfd, buffer, 1000);
									if (n < 0){
										error("ERROR writing to socket");
										close(conn_soc[i].sockfd);
										conn_soc[i].sockfd = -1;
										conn_soc[i].login_id = -100 ;
									}
									printf("4 done\n");
									break;
								//5 is so that trader can logout from server.
								case '5' :
									//socket is closed for that trader.
									n = write(conn_soc[i].sockfd, "1", 1);
									close(conn_soc[i].sockfd);
									conn_soc[i].sockfd = -1;
									conn_soc[i].login_id = -100 ;
									printf("5 done\n");
									break;
								default:
									break;
							}
						}
					}
				}
				break;
		}
	}
	return 0;
}