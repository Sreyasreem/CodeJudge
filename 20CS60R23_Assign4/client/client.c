/*
THE CLIENT PROGRAM

NAME- SREYASREE MANDAL
ROLL NO. 20CS60R23
ASSIGNMENT NO. 4

COMPILE: 
gcc client.c -o client

RUN:
./client localhost <PORT NUMBER>

RUN example:
./client localhost 8080

The command line arguments are as follows:
argv[1] - localhost
argv[2] - <PORT NUMBER>

CODEJUD COMMAND FORMAT:
CODEJUD <filename> c/cpp

Example:
CODEJUD add.cpp cpp
CODEJUD div.c c
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include<netdb.h>
#define MAXCHAR 2048

/* Function declarations */

/*Store File received from the server */
void retr_file(int sockfd, char file_name[]);

/*Send File to the server */
void send_file(int sockfd, char file_name[]);

/*Receive code review from server online judge */
void codejudge_file(int sockfd);

/*Check whether a File already exists at the server directory */
int file_existence(char file_name[]);

/*Count the number of tokens in the command received from the client */
int count_tokens(char buffer[]);

int main(int argc, char * argv[]) {

  int sock, server_resp;
  struct sockaddr_in serv_addr;
  char buffer[MAXCHAR] = {0};
  char storage[MAXCHAR] = {0};
  struct hostent * server;
  int number_of_tokens;
  int PORT;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("\n Socket creation error \n");
    return -1;
  }

  //Initializes buffer
  bzero( & serv_addr, sizeof(serv_addr));

  //Resolves host address
  server = gethostbyname(argv[1]);
  PORT = atoi(argv[2]);

  serv_addr.sin_family = AF_INET;
  bcopy((char * ) server -> h_addr, (char * ) & serv_addr.sin_addr.s_addr, server -> h_length);
  serv_addr.sin_port = htons(PORT);

  //Initiating connection request to the server
  if (connect(sock, (struct sockaddr * ) & serv_addr, sizeof(serv_addr)) < 0) {
    printf("\nConnection Failed \n");
    return -1;
  }

  printf("\nConnected to server\n");

  while (1) {
    bzero(buffer, MAXCHAR);
    bzero(storage, MAXCHAR);
    printf("\nPlease enter the message to the server: ");
    fgets(buffer, MAXCHAR, stdin);
    int len = strlen(buffer);
    buffer[len - 1] = '\0';
    strcpy(storage, buffer);

    number_of_tokens = count_tokens(buffer);

    write(sock, storage, sizeof(storage));

    //Break the buffer string by space and extract command from the client 
    char * command = strtok(buffer, " ");
    char * file_name = strtok(NULL, " ");

    if ((number_of_tokens == 1) && !strcmp(command, "QUIT")) {
      close(sock);
      printf("\nDisconnected from server \n");
      exit(1);
    } else if ((number_of_tokens == 2) && !strcmp(command, "RETR"))
      retr_file(sock, file_name);

    else if ((number_of_tokens == 2) && !strcmp(command, "STOR"))
      send_file(sock, file_name);

    else if ((number_of_tokens == 3) && !strcmp(command, "CODEJUD")) {

      read(sock, buffer, sizeof(buffer));

      //valid command
      if (!strcmp(buffer, "1")) {
        send_file(sock, file_name);
        codejudge_file(sock);
      }

      //file extensions mismatch   
      else
        printf("\n%s", buffer);
    } else {
      read(sock, buffer, sizeof(buffer));
      printf("\n%s", buffer);
    }
  }
  return 0;
}

/*Store File received from the server */
void retr_file(int sockfd, char file_name[]) {

  FILE * fp;
  char buffer[MAXCHAR] = {0};

  //File already exists at the client directory. The RETR command is stopped
  if (file_existence(file_name)) {
    strcpy(buffer, "Sorry! File already exists at the client directory.");
    printf("\n%s\n", buffer);
    write(sockfd, buffer, sizeof(buffer));
  }

  //File does not exist at client directory. 
  else {
    strcpy(buffer, "1");

    //Conveys the message to server to send file
    write(sockfd, buffer, sizeof(buffer));
    bzero(buffer, MAXCHAR);

    fp = fopen(file_name, "w");

    while (1) {
      read(sockfd, buffer, sizeof(buffer));

      //Either the file does not exist at the server directory or file transmission is completed
      if (!strcmp(buffer, "") || !strcmp(buffer, "No such file exists at the server directory!"))
        break;
      fprintf(fp, "%s", buffer);
      fflush(fp);
      bzero(buffer, MAXCHAR);
    }

    fclose(fp);

    //Requested file unavailable at server directory
    if (!strcmp(buffer, "No such file exists at the server directory!")) {

      //Delete the empty file that was created to store file data  
      remove(file_name);
      printf("\nServer Message : %s\n", buffer);
    } else
      printf("\nReceived file %s\n", file_name);
  }
}

/*Send File to the server on STOR command*/
void send_file(int sockfd, char file_name[]) {

  FILE * fp;
  char buffer[MAXCHAR] = {0};

  read(sockfd, buffer, sizeof(buffer));
  if (!strcmp(buffer, "File already exists at the server directory!"))
    printf("\nServer message : %s\n", buffer);

  else {
    //File unavailable at the client directory
    if (!file_existence(file_name)) {
      strcpy(buffer, "No such file exists at the client directory!");
      printf("\n%s\n", buffer);
      write(sockfd, buffer, sizeof(buffer));
    }
    //File available at the client directory
    else {
      fp = fopen(file_name, "r");
      printf("\nSending file %s to server\n", file_name);

      //Keeps sending the data of the file till the end of the file
      while (fgets(buffer, MAXCHAR, fp) != NULL) {

        write(sockfd, buffer, sizeof(buffer));
        bzero(buffer, MAXCHAR);
      }
      fclose(fp);

      //Mark the end of the file reading
      strcpy(buffer, "");
      write(sockfd, buffer, sizeof(buffer));
    }
  }
}

/*Receive code review from server online judge */
void codejudge_file(int sockfd) {

  int input_flag = 0;
  int error_flag = 0;
  FILE * fp;
  char buffer[MAXCHAR] = {0};
  char exec_result[MAXCHAR] = {0};

  printf("\nReceiving code review from Online Judge...\n\n");

  //result of compilation phase
  while (1) {
    read(sockfd, buffer, sizeof(buffer));

    //Either the file is empty or file transmission is completed
    if (!strcmp(buffer, "") || !strcmp(buffer, "\nCOMPILE_SUCCESS - The judge’s compiler compile code successfully.\n")) {
      printf("%s", buffer);
      break;
    }

    printf("%s", buffer);
    bzero(buffer, MAXCHAR);
  }

  if (!strcmp(buffer, "\nCOMPILE_SUCCESS - The judge’s compiler compile code successfully.\n")) {

    //result of execution phase
    read(sockfd, exec_result, sizeof(exec_result));

    //input file required at server side
    if (!strcmp(exec_result, "i"))
      input_flag = 1;

    //no input file required at server side   
    else
      input_flag = 2;

    while (1) {
      bzero(exec_result, MAXCHAR);
      read(sockfd, exec_result, sizeof(exec_result));

      if (!strcmp(exec_result, ""))
        break;

      else if (!strcmp(exec_result, "\nRUN_SUCCESS - The judge find no error during program execution(run-time).\n")) {
        printf("%s", exec_result);

        //input file required at the server side
        if (input_flag == 1) {
          read(sockfd, exec_result, sizeof(exec_result));
          printf("%s", exec_result);
        }

      } else {
        printf("%s", exec_result);
        if (input_flag == 2)
          error_flag = 1;
      }

    }
  }

  //no input file required at the server side and no runtime error occurred
  if (input_flag == 2 && !error_flag) {

    //result of matching phase
    while (1) {
      bzero(buffer, MAXCHAR);
      read(sockfd, buffer, sizeof(buffer));

      //data transmission completed
      if (!strcmp(buffer, ""))
        break;

      printf("%s", buffer);
    }
  }
}

/*Check whether a File already exists at the server directory */
int file_existence(char file_name[]) {

  FILE * fp;
  int exist = 0;

  //Trying to read the file
  if (fp = fopen(file_name, "r")) {
    fclose(fp);

    //On success, returns 1
    exist = 1;
  }
  return exist;
}

/*Count the number of tokens in the command received from the client */
int count_tokens(char buffer[]) {

  char storage[MAXCHAR] = {0};
  int number_of_tokens = 0;

  strcpy(storage, buffer);

  //first token: space is the delimiter 
  char * token = strtok(storage, " ");

  // Keep counting tokens till it gets NULL 
  while (token != NULL) {
    number_of_tokens++;
    token = strtok(NULL, " ");
  }
  return number_of_tokens;
}
