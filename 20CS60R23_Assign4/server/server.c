/*
THE SERVER PROGRAM

NAME- SREYASREE MANDAL
ROLL NO. 20CS60R23
ASSIGNMENT NO. 4

COMPILE: 
gcc server.c -o server

RUN:
./server <PORT NUMBER>

RUN example:
./server 8080

The command line arguments are as follows:
argv[1] - <PORT NUMBER>
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <dirent.h>
#define MAXCHAR 2048

/* Function declarations */

/*Send File to the client on RETR command */
void send_file(int sockfd, char file_name[], char addr[], int port);

/*Store File received from the client on STOR command */
void store_file(int sockfd, char file_name[], char addr[], int port);

/*Send all the contents of the directory to client on LIST command */
void list_files(int sockfd, char addr[], int port);

/*Delete a File received from the client on DELE command */
void delete_file(int sockfd, char file_name[], char addr[], int port);

/*Compile and execute the file and match the output with test cases */
void codejudge_file(int sockfd, char file_name[], char file_ext[], char addr[], int port);

/* Compares results of testfile with actual output file */
void outputFileMatching(int sockfd, char file_name[]);

/*Check whether a File already exists at the server directory */
int file_existence(char file_name[]);

/*Count the number of tokens in the command received from the client */
int count_tokens(char buffer[]);

int main(int argc, char * argv[]) {

  int new_socket, addrlen, master_socket_communication,
  client_communication, index, client_message;
  int sockd, max_sockd, PORT, number_of_tokens;
  int max_clients = 50;
  int client_socket[50];
  char * extension;
  struct sockaddr_in address;
  char buffer[MAXCHAR] = {0};
  char temp_buffer[MAXCHAR] = {0};
  FILE * fp;

  //set of socket descriptors
  fd_set main_fd;

  PORT = atoi(argv[1]);

  //initialise all client_sockets to 0
  for (index = 0; index < max_clients; index++) {
    client_socket[index] = 0;
  }

  //create a master socket
  if ((master_socket_communication = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  //type of socket created
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  //bind the socket to localhost port 
  if (bind(master_socket_communication, (struct sockaddr * ) & address, sizeof(address)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  //maximum of 5 pending connections for the master socket
  if (listen(master_socket_communication, 5) < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  //accept the incoming connection
  addrlen = sizeof(address);

  //clear the socket set
  FD_ZERO( & main_fd);

  //add master socket to set
  FD_SET(master_socket_communication, & main_fd);
  max_sockd = master_socket_communication;

  while (1) {
    fd_set readfds = main_fd;

    //add child sockets to set
    for (index = 0; index < max_clients; index++) {
      //socket descriptor
      sockd = client_socket[index];

      //add to read list
      if (sockd > 0)
        FD_SET(sockd, & readfds);

      //maximum file descriptor number
      if (sockd > max_sockd)
        max_sockd = sockd;
    }

    //Implementation of select system call to accept connections in a non-blocking mode
    client_communication = select(max_sockd + 1, & readfds, NULL, NULL, NULL);

    if ((client_communication < 0) && (errno != EINTR)) {
      printf("Error in the select system call");
    }

    //accept new connection
    if (FD_ISSET(master_socket_communication, & readfds)) {
      if ((new_socket = accept(master_socket_communication, (struct sockaddr * ) & address, (socklen_t * ) & addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
      }

      printf("\nConnected with client socket number %s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

      //add new socket to array of sockets
      for (index = 0; index < max_clients; index++) {
        //check empty position
        if (client_socket[index] == 0) {
          client_socket[index] = new_socket;
          break;
        }
      }
    }

    //Perform IO operation on some other socket
    for (index = 0; index < max_clients; index++) {
      sockd = client_socket[index];

      if (FD_ISSET(sockd, & readfds)) {

        // If connection is established then start communicating 
        client_message = read(sockd, buffer, MAXCHAR);
        getpeername(sockd, (struct sockaddr * ) & address, (socklen_t * ) & addrlen);
        printf("\nClient socket %s:%d sent message: %s \n", inet_ntoa(address.sin_addr), ntohs(address.sin_port), buffer);

        number_of_tokens = count_tokens(buffer);

        //Break the buffer string by space and extract command from the client 
        char * command = strtok(buffer, " ");
        char * file_name = strtok(NULL, " ");
        char * file_ext = strtok(NULL, " ");

        //Implementation of FTP commands
        if (number_of_tokens == 1) {

          if (!strcmp(command, "QUIT")) {
            printf("\n%s command received from client %s:%d \n", command, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
            close(sockd);
            client_socket[index] = 0;
          } else if (!strcmp(command, "LIST"))
            list_files(sockd, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

          else {
            strcpy(buffer, "INVALID COMMAND!!!\n");
            printf("\nSending reply to client %s:%d : %s\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port), buffer);
            write(sockd, buffer, sizeof(buffer));
          }

        } else if (number_of_tokens == 2) {

          if (!strcmp(command, "DELE"))
            delete_file(sockd, file_name, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

          else if (!strcmp(command, "RETR"))
            send_file(sockd, file_name, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

          else if (!strcmp(command, "STOR"))
            store_file(sockd, file_name, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

          else {
            strcpy(buffer, "INVALID COMMAND!!!\n");
            printf("\nSending reply to client %s:%d : %s\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port), buffer);
            write(sockd, buffer, sizeof(buffer));
          }
        } else if (number_of_tokens == 3) {

          strcpy(temp_buffer, file_name);
          char * extension = strtok(temp_buffer, ".");
          extension = strtok(NULL, ".");

          //valid command only if 3rd token matches with the file extension of 2nd token
          if (!strcmp(command, "CODEJUD") && !strcmp(file_ext, extension)) {

            //to indicate a valid command
            strcpy(buffer, "1");
            write(sockd, buffer, sizeof(buffer));

            store_file(sockd, file_name, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
            codejudge_file(sockd, file_name, file_ext, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
          } else {
            strcpy(buffer, "INVALID COMMAND!!!\n");
            printf("\nSending reply to client %s:%d : %s\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port), buffer);
            write(sockd, buffer, sizeof(buffer));
          }
        } else {
          strcpy(buffer, "INVALID COMMAND!!!\n");
          printf("\nSending reply to client %s:%d : %s\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port), buffer);
          write(sockd, buffer, sizeof(buffer));
        }
      }
    }
  }

  return 0;
}

/*Send File to the client on RETR command */
void send_file(int sockfd, char file_name[], char addr[], int port) {

  FILE * fp;
  char buffer[MAXCHAR] = {0};

  read(sockfd, buffer, sizeof(buffer));

  //File already exists at the client directory. The RETR command is rejected
  if (!strcmp(buffer, "Sorry! File already exists at the client directory."))
    printf("\nClient apology message : %s\n", buffer);

  else {
    //The file requested by client unavailable at the server directory
    if (!file_existence(file_name)) {
      strcpy(buffer, "No such file exists at the server directory!");
      printf("\n%s\n", buffer);
      write(sockfd, buffer, sizeof(buffer));
    }
    //The file requested by client available at the server directory
    else {
      fp = fopen(file_name, "r");
      printf("\nSending file %s to client %s:%d\n", file_name, addr, port);

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

/*Store File received from the client on STOR command */
void store_file(int sockfd, char file_name[], char addr[], int port) {

  FILE * fp;
  char buffer[MAXCHAR] = {0};

  //File already exists at the server directory. The STOR command is stopped
  if (file_existence(file_name)) {
    strcpy(buffer, "File already exists at the server directory!");
    printf("\n%s\n", buffer);
    write(sockfd, buffer, sizeof(buffer));
  }

  //File does not exist at the server directory. 
  else {
    strcpy(buffer, "1");

    //Conveys the message to the client to send file
    write(sockfd, buffer, sizeof(buffer));
    bzero(buffer, MAXCHAR);

    fp = fopen(file_name, "w");

    while (1) {
      read(sockfd, buffer, sizeof(buffer));

      //Either the file does not exist at the client directory or file transmission is completed
      if (!strcmp(buffer, "") || !strcmp(buffer, "No such file exists at the client directory!"))
        break;
      fprintf(fp, "%s", buffer);
      fflush(fp);
      bzero(buffer, MAXCHAR);
    }

    fclose(fp);

    //File does not exist at the client directory
    if (!strcmp(buffer, "No such file exists at the client directory!")) {

      //delete the empty file that was created to store file data  
      remove(file_name);
      printf("\nClient apology message  : %s\n", buffer);
    } else
      printf("\nStored file %s received from the client %s:%d\n", file_name, addr, port);
  }
}

/*Send all the contents of the directory to client on LIST command */
void list_files(int sockfd, char addr[], int port) {

  char output[2048] = {0};
  DIR * current_directory;
  struct dirent * directory_ptr;

  current_directory = opendir(".");
  if (current_directory) {
    //Keep reading the contents of the server directory and concatenate them into a string
    while ((directory_ptr = readdir(current_directory)) != NULL) {
      strcat(output, directory_ptr -> d_name);
      strcat(output, "\n");
    }
    closedir(current_directory);
  }
  printf("\nSending list of files to client %s:%d \n", addr, port);
  write(sockfd, output, sizeof(output));
}

/*Delete a File received from the client on DELE command */
void delete_file(int sockfd, char file_name[], char addr[], int port) {

  char buffer[MAXCHAR] = {0};

  //File present at server directory and sucessfully removed
  if (remove(file_name) == 0) {
    strcat(buffer, file_name);
    strcat(buffer, " deleted successfully.\n");
  }
  //File unavailable at server directory
  else {
    strcat(buffer, file_name);
    strcat(buffer, " unavailable for deletion.\n");
  }

  printf("\nSending reply to client %s:%d : %s\n", addr, port, buffer);
  write(sockfd, buffer, sizeof(buffer));
}

/*Compile and execute the file and match the output with test cases */
void codejudge_file(int sockfd, char file_name[], char file_ext[], char addr[], int port) {

  int execution_time;
  int test_len, output_len;
  char * source;
  FILE * fp;
  FILE * fp_input;
  FILE * fp_test;
  FILE * fp_output;
  FILE * fp_temp;

  char buffer[MAXCHAR] = {0};
  char output_buffer[MAXCHAR] = {0};
  char test_buffer[MAXCHAR] = {0};
  char command[MAXCHAR] = {0};
  char storage[MAXCHAR] = {0};
  char input_file[MAXCHAR] = {0};
  char testcase_file[MAXCHAR] = {0};
  char output_file[MAXCHAR] = {0};

  strcpy(storage, file_name);
  source = strtok(storage, ".");

  /* Compilation Phase */

  //c file
  if (!strcmp(file_ext, "c"))
    sprintf(command, "gcc %s -o %s 2> compilationFile.txt", file_name, source);

  //c++ file  
  else
    sprintf(command, "g++ %s -o %s 2> compilationFile.txt", file_name, source);

  system(command);

  fp = fopen("compilationFile.txt", "r");
  printf("\nSending code review of the file %s to client %s:%d\n", file_name, addr, port);

  //goto end of file to check if empty or not
  fseek(fp, 0, SEEK_END);

  //empty file: compilation is successful
  if (ftell(fp) == 0) {
    strcpy(buffer, "\nCOMPILE_SUCCESS - The judge’s compiler compile code successfully.\n");
    printf("%s", buffer);
    write(sockfd, buffer, sizeof(buffer));

    sprintf(input_file, "input_%s.txt", source);
    sprintf(testcase_file, "testcase_%s.txt", source);
    sprintf(output_file, "output_%s.txt", source);

    /* Execution Phase */

    //file sent by client requires an input file during runtime
    if (file_existence(input_file)) {

      bzero(buffer, MAXCHAR);
      strcpy(buffer, "i");
      write(sockfd, buffer, sizeof(buffer));

      fp_input = fopen(input_file, "r");
      fp_test = fopen(testcase_file, "r");
      fp_output = fopen(output_file, "w");
      fclose(fp_output);
      fp_output = fopen(output_file, "r");

      bzero(buffer, MAXCHAR);
      int index = 0;
      while (fgets(buffer, MAXCHAR, fp_input) != NULL) {

        index++;
        bzero(command, MAXCHAR);

        //create a temporary file to store the current inputs from
        fp_temp = fopen("tempInputFile.txt", "w");
        fprintf(fp_temp, "%s", buffer);
        fflush(fp_temp);
        fclose(fp_temp);

        //prepare a string for code execution
        sprintf(command, "timeout --preserve-status 1 ./%s 0< tempInputFile.txt 1>> output_%s.txt", source, source);
        execution_time = system(command);

        bzero(buffer, MAXCHAR);

        if (execution_time == 0) {
          sprintf(buffer, "\nRUN_SUCCESS TESTCASE_%d - The judge find no error during program execution(run-time).\n", index);
          printf("%s", buffer);
          write(sockfd, buffer, sizeof(buffer));

          //matching phase of each test case with the actual output
          bzero(test_buffer, MAXCHAR);
          fgets(test_buffer, MAXCHAR, fp_test);
          test_buffer[strcspn(test_buffer, "\r\n")] = 0;
          test_len = strlen(test_buffer);

          for (int index = test_len - 1; index >= 0; index--) {
            if (test_buffer[index] == '\n' || test_buffer[index] == '\r' || test_buffer[index] == '\t' || test_buffer[index] == ' ') {
              test_buffer[index] = 0;
              continue;
            }
            break;
          }

          bzero(output_buffer, MAXCHAR);
          fgets(output_buffer, MAXCHAR, fp_output);
          output_buffer[strcspn(output_buffer, "\r\n")] = 0;
          output_len = strlen(output_buffer);

          for (int index = output_len - 1; index >= 0; index--) {
            if (output_buffer[index] == '\n' || output_buffer[index] == '\r' || output_buffer[index] == '\t' || output_buffer[index] == ' ') {
              output_buffer[index] = 0;
              continue;
            }
            break;
          }

          if (!strcmp(test_buffer, output_buffer)) {
            sprintf(output_buffer, "\nTESTCASE_%d PASSED\n", index);
            printf("%s", output_buffer);

          } else {
            sprintf(output_buffer, "\nTESTCASE_%d FAILED\n", index);
            printf("%s", output_buffer);
          }

          write(sockfd, output_buffer, sizeof(buffer));
        } else {
          if (execution_time == 36608) {
            strcpy(buffer, "\nTIME LIMIT EXCEEDED - Client’s program failed to finish executing before the established time limit (1 sec) for the problem.\n");
            printf("%s", buffer);
          } else {
            sprintf(buffer, "\nRUN_ERROR TESTCASE_%d - The judge find error occurs during program execution(run-time).\n", index);
            printf("%s", buffer);
          }
          write(sockfd, buffer, sizeof(buffer));
        }
        bzero(buffer, MAXCHAR);
      }
      bzero(buffer, MAXCHAR);
      strcpy(buffer, "");
      write(sockfd, buffer, sizeof(buffer));
      remove("tempInputFile.txt");
      fclose(fp_test);

      //outputfile is empty
      fseek(fp_output, 0, SEEK_END);
      if (ftell(fp_output) == 0) {
        fclose(fp_output);
        remove(output_file);
      }
    }

    //no input file required during runtime
    else {
      bzero(buffer, MAXCHAR);
      strcpy(buffer, "n");
      write(sockfd, buffer, sizeof(buffer));
      bzero(buffer, MAXCHAR);
      bzero(command, MAXCHAR);

      //prepare a string for code execution
      sprintf(command, "timeout --preserve-status 1 ./%s 1> output_%s.txt", source, source);
      execution_time = system(command);
      if (execution_time == 0) {
        strcpy(buffer, "\nRUN_SUCCESS - The judge find no error during program execution(run-time).\n");
        printf("%s", buffer);
        write(sockfd, buffer, sizeof(buffer));
      } else {
        if (execution_time == 36608)
          strcpy(buffer, "\nTIME LIMIT EXCEEDED - Client’s program failed to finish executing before the established time limit (1 sec) for the problem.\n");

        else
          strcpy(buffer, "\nRUN_ERROR - The judge find error occurs during program execution(run-time).\n");

        printf("%s", buffer);
        write(sockfd, buffer, sizeof(buffer));
        remove(output_file);
      }
      bzero(buffer, MAXCHAR);
      strcpy(buffer, "");
      write(sockfd, buffer, sizeof(buffer));

      if (execution_time == 0)
        outputFileMatching(sockfd, source);
    }
  }

  //compilation phase contains errors  
  else {
    bzero(buffer, MAXCHAR);
    strcpy(buffer, "\nCOMPILE_ERROR - The judge’s compiler can not compile client’s source code.\n");
    printf("%s", buffer);
    write(sockfd, buffer, sizeof(buffer));
    bzero(buffer, MAXCHAR);

    //goto beginning of the file
    fseek(fp, 0, SEEK_SET);

    //Keeps sending compilation errors till the end of the file
    while (fgets(buffer, MAXCHAR, fp) != NULL) {

      write(sockfd, buffer, sizeof(buffer));
      bzero(buffer, MAXCHAR);
    }

    //Mark the end of the file reading
    strcpy(buffer, "");
    write(sockfd, buffer, sizeof(buffer));
  }

  fclose(fp);

  //remove the compilation error file
  remove("compilationFile.txt");

  //remove the files after code review
  remove(file_name);
  remove(source);
}

/* Compares results of testfile with actual output file */
void outputFileMatching(int sockfd, char file[]) {

  FILE * fp_test;
  FILE * fp_output;
  char testFile[64] = {0};
  char outputFile[64] = {0};
  char buffer[MAXCHAR] = {0};
  char test_buffer[MAXCHAR] = {0};
  char output_buffer[MAXCHAR] = {0};
  int index = 0;
  int test_len;
  int output_len;

  sprintf(testFile, "testcase_%s.txt", file);
  sprintf(outputFile, "output_%s.txt", file);

  fp_test = fopen(testFile, "r");
  fp_output = fopen(outputFile, "r");

  //performs matching of test file and actual output file
  while ((fgets(test_buffer, MAXCHAR, fp_test) != NULL)) {

    index++;
    if ((fgets(output_buffer, MAXCHAR, fp_output) != NULL)) {

      //remove trailing spaces
      test_buffer[strcspn(test_buffer, "\r\n")] = 0;
      test_len = strlen(test_buffer);

      for (int index = test_len - 1; index >= 0; index--) {
        if (test_buffer[index] == '\n' || test_buffer[index] == '\r' || test_buffer[index] == '\t' || test_buffer[index] == ' ') {
          test_buffer[index] = 0;
          continue;
        }
        break;
      }

      output_buffer[strcspn(output_buffer, "\r\n")] = 0;
      output_len = strlen(output_buffer);

      for (int index = output_len - 1; index >= 0; index--) {
        if (output_buffer[index] == '\n' || output_buffer[index] == '\r' || output_buffer[index] == '\t' || output_buffer[index] == ' ') {
          output_buffer[index] = 0;
          continue;
        }
        break;
      }

      if (!strcmp(test_buffer, output_buffer)) {
        sprintf(buffer, "\nTESTCASE_%d PASSED\n", index);
        printf("%s", buffer);
      } else {
        sprintf(buffer, "\nTESTCASE_%d FAILED\n", index);
        printf("%s", buffer);
      }

    } else {
      sprintf(buffer, "\nTESTCASE_%d FAILED\n", index);
      printf("%s", buffer);
    }

    write(sockfd, buffer, sizeof(buffer));
    bzero(buffer, MAXCHAR);
    bzero(test_buffer, MAXCHAR);
    bzero(output_buffer, MAXCHAR);
  }
  strcpy(buffer, "");
  write(sockfd, buffer, sizeof(buffer));

  fclose(fp_test);

  //outputfile is empty
  fseek(fp_output, 0, SEEK_END);
  if (ftell(fp_output) == 0) {
    fclose(fp_output);
    remove(outputFile);
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
