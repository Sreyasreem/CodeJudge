/*
THE CLIENT PROGRAM

NAME- SREYASREE MANDAL
ROLL NO. 20CS60R23
ASSIGNMENT NO. 2
PROBLEM 2

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
#include <stdlib.h>
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include<netdb.h>
#define MAXCHAR 1024

int main(int argc, char *argv[]) {

    int sock, server_resp; 
    struct sockaddr_in serv_addr;  
    char buffer[MAXCHAR]; 
    struct hostent *server;  
    int PORT;
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
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
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    }
    
    printf("\nConnected to server\n");
    
    while(1){
            
	    bzero(buffer, MAXCHAR);
	    printf("\nPlease enter the message to the server: ");
	    fgets(buffer, MAXCHAR, stdin); 
	    int len = strlen(buffer);
	    buffer[len-1]='\0';
	    
	    write(sock , buffer , sizeof(buffer)); 
	    
	    if ( !strcmp(buffer, "exit") ){
	       close(sock); 
               printf("\nDisconnected from the server.\n");
               exit(1);
             } 
             
	    bzero(buffer, MAXCHAR); 
	    server_resp = read( sock , buffer, MAXCHAR); 
	    printf("\nServer replied: %s\n",buffer );
         } 
return 0;
}
