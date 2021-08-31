/*
THE CLIENT PROGRAM

NAME- SREYASREE MANDAL
ROLL NO. 20CS60R23
ASSIGNMENT NO. 1
PROBLEM 1

COMPILE: 
gcc client.c -o client

RUN:
./client localhost <PORT NUMBER>

RUN example:
./client localhost 8080

The command line arguments are as follows:
argv[1] - localhost
argv[2] - <PORT NUMBER>
*/

#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include<netdb.h>
#include<stdlib.h>

int main(int argc, char *argv[]) {

    int sockfd, server_value; 
    struct sockaddr_in serv_addr; 
    char buffer[1024];
    int PORT;
    struct hostent *server;  
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
    
    //Initializes buffer
    bzero(&serv_addr, sizeof(serv_addr)); 
    
    //Resolves host address
    server = gethostbyname(argv[1]);
    PORT = atoi(argv[2]);
    
    serv_addr.sin_family = AF_INET; 
    bcopy((char *)server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length); 
    serv_addr.sin_port = htons(PORT);
    
    //Initiating connection request to the server
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    }
    
    bzero(buffer, 1024);
    printf("Enter your message: ");
    
    //User input
    fgets(buffer, 1024, stdin);
    
    //Sending message to server 
    write(sockfd , buffer , sizeof(buffer)); 
    
    bzero(buffer, 1024);
    
    //Receiving acknowledgement message from server 
    server_value = read( sockfd , buffer, 1024); 
    printf("\nMessage from the Server: %s\n",buffer );
    
    close(sockfd);
    
return 0;
}
