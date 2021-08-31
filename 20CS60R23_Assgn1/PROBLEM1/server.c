/*
THE SERVER PROGRAM

NAME- SREYASREE MANDAL
ROLL NO. 20CS60R23
ASSIGNMENT NO. 1
PROBLEM 1

COMPILE: 
gcc server.c -o server

RUN:
./server <PORT NUMBER>

RUN example:
./server 8080

The command line arguments are as follows:
argv[1] - <PORT NUMBER>

*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define MAXCHAR 1024

int main(int argc, char * argv[]) {

  int sockfd, new_socket, client_value;
  struct sockaddr_in serv_addr;
  int addrlen = sizeof(serv_addr);
  char buffer[MAXCHAR];
  int PORT;

  // Creating socket file descriptor 
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  // Initialize socket structure 
  bzero((char * ) & serv_addr, sizeof(serv_addr));

  PORT = atoi(argv[1]);

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(PORT);

  // Attaching socket to the port number
  if (bind(sockfd, (struct sockaddr * ) & serv_addr, sizeof(serv_addr)) < 0) {
    perror("Bind Failed!!!");
    exit(EXIT_FAILURE);
  }

  //Listening for connection
  if (listen(sockfd, 3) < 0) {
    perror("Listening...");
    exit(EXIT_FAILURE);
  }

  while (1) {

    //Client connection acceptance
    if ((new_socket = accept(sockfd, (struct sockaddr * ) & serv_addr, (socklen_t * ) & addrlen)) < 0) {
      perror("accept");
      exit(EXIT_FAILURE);
    }

    //If connection is established, communication begins 
    bzero(buffer, MAXCHAR);

    //Reading message from client into the buffer
    client_value = read(new_socket, buffer, MAXCHAR);
    printf("\nMessage received from the Client: %s\n", buffer);

    strcpy(buffer, "Hello! Thank you for your message.");

    //Sending acknowledgment message to client
    write(new_socket, buffer, sizeof(buffer));
  }
  
  close(new_socket);
  close(sockfd);

  return 0;
}
