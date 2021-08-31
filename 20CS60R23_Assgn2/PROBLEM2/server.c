/*
THE SERVER PROGRAM

NAME- SREYASREE MANDAL
ROLL NO. 20CS60R23
ASSIGNMENT NO. 2
PROBLEM 2

COMPILE: 
gcc server.c -lm -o server

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
#include <math.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <time.h>
#define MAXCHAR 1024

char server_reply[MAXCHAR] = {0};

/* Function declarations */

/* Defines the operator precedence. Higher the value, higher is the precedence */
int precedence(char op); 

/* Perform standard mathematical operation */
double expCalculation(double val1, double val2, char operator); 

/* Evaluate the value of a postfix expression */
double evaluatePostfix(char buffer[]); 

/* Check the validity of an infix expression */
double isValidInfixForm(char str[]);

/* Conversion of Infix expression into Postfix expression */
int infixToPostfixConversion(char str[], char cli_addr[], int cli_port);

int main(int argc, char * argv[]) {

  int master_socket, addrlen, new_socket, client_socket[50],
    max_clients = 50, activity, i, valread, sockd;
  int max_sockd;
  struct sockaddr_in address;
  int PORT;
  char buffer[MAXCHAR] = {0};
  FILE * fp;
  time_t start_time, end_time;
  int success;

  //set of socket descriptors
  fd_set main_fd;

  PORT = atoi(argv[1]);
  
  //create the text file if does not exist
  fp = fopen("server_records.txt", "w");

  if (fp == NULL) {
    printf("Error!");
    exit(1);
  }

  //initialise all client_sockets to 0
  for (i = 0; i < max_clients; i++) {
    client_socket[i] = 0;
  }

  //create a master socket
  if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  //type of socket created
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  //bind the socket to localhost port 
  if (bind(master_socket, (struct sockaddr * ) & address, sizeof(address)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  //maximum of 5 pending connections for the master socket
  if (listen(master_socket, 5) < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  //accept the incoming connection
  addrlen = sizeof(address);

  //clear the socket set
  FD_ZERO( & main_fd);

  //add master socket to set
  FD_SET(master_socket, & main_fd);
  max_sockd = master_socket;

  while (1) {
    fd_set readfds = main_fd;

    //add child sockets to set
    for (i = 0; i < max_clients; i++) {
      //socket descriptor
      sockd = client_socket[i];

      //add to read list
      if (sockd > 0)
        FD_SET(sockd, & readfds);

      //maximum file descriptor number
      if (sockd > max_sockd)
        max_sockd = sockd;
    }

    //Use of select as non-blocking system call
    activity = select(max_sockd + 1, & readfds, NULL, NULL, NULL);

    if ((activity < 0) && (errno != EINTR)) {
      printf("select error");
    }

    //accept new connection
    if (FD_ISSET(master_socket, & readfds)) {
      if ((new_socket = accept(master_socket, (struct sockaddr * ) & address, (socklen_t * ) & addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
      }

      printf("\nConnected with client socket number %s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
      time(&start_time);

      //add new socket to array of sockets
      for (i = 0; i < max_clients; i++) {
        //check empty position
        if (client_socket[i] == 0) {
          client_socket[i] = new_socket;
          break;
        }
      }
    }

    //else its some IO operation on some other socket
    for (i = 0; i < max_clients; i++) {
      sockd = client_socket[i];

      if (FD_ISSET(sockd, & readfds)) {

        // If connection is established then start communicating 
        valread = read(sockd, buffer, MAXCHAR);

        if (!strcmp(buffer, "exit")) {
          printf("\nThe client %s:%d has exited.\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
          close(sockd);
          client_socket[i] = 0;
        } else {

          getpeername(sockd, (struct sockaddr * ) & address, (socklen_t * ) & addrlen);
          printf("\nClient socket %s:%d sent message: %s \n", inet_ntoa(address.sin_addr), ntohs(address.sin_port), buffer);

          success = infixToPostfixConversion(buffer, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

          bzero(buffer, MAXCHAR);

          //expression is valid
          if (success) {
            printf("\nSending reply: %s\n", server_reply);
            strcpy(buffer, server_reply);
            
            time(&end_time);

    	     fprintf(fp, "Time Taken = %.2f seconds\n", difftime(end_time, start_time));
             fflush(fp);
          }

          //expression is invalid
          else {
            printf("\nSending reply: INVALID EXPRESSION!!!\n");
            strcpy(buffer, "INVALID EXPRESSION!!!");
          }

          //sending reply to client
          write(sockd, buffer, MAXCHAR);
        }
      }
    }
  }

  return 0;
}

/* Defines the operator precedence. Higher the value, higher is the precedence */
int precedence(char op) {

  int value = 10;

  if (op == '(')
    value = 0;

  else if (op == '+' || op == '-')
    value = 1;

  else if (op == '*' || op == '/' || op == '%')
    value = 2;

  else if (op == '#')
    value = 3;

  else if (op == '^')
    value = 4;

  return value;
}

/* Perform standard mathematical operation */
double expCalculation(double val1, double val2, char operator) {

  double value = -191919.191919;

  switch (operator) {
  case '+':
    value = val1 + val2;
    break;

  case '-':
    value = val1 - val2;
    break;

  case '*':
    value = val1 * val2;
    break;

  case '/':
  
    if( val2 == 0.0)
      break;
      
    value = val1 / val2;
    break;

  case '%':
  
    if( val2 == 0.0)
      break;
      
    value = fmod(val1, val2);
    break;

  case '^':
    value = pow(val1, val2);
    break;

  default:
    printf("\nInvalid operator!!!\n");
  }

  return value;
}

/* Evaluate the value of a postfix expression */
double evaluatePostfix(char buffer[]) {

  double value;
  int TOP = -1;
  int len = strlen(buffer);
  double stack[MAXCHAR] = {0}; 

  char * token = strtok(buffer, " ");

  while (token != NULL) {
    //push the number to the stack
    if (isdigit(token[0]))
      stack[++TOP] = atof(token);

    else {
      //unary minus calculation
      if (token[0] == '#')
        stack[TOP] = stack[TOP] * (-1);

      else {
        //calculation with binary operator
        value = expCalculation(stack[TOP - 1], stack[TOP], token[0]);
        stack[--TOP] = value;
      }
    }
    token = strtok(NULL, " ");
  }
  value = stack[TOP];

  return value;
}

/* Check the validity of an infix expression */
double isValidInfixForm(char str[]) {

  int valid = 1;
  int flag = 0;
  int len = strlen(str);

  //TOP of operator stack
  int TOP_ops = -1;

  //TOP of value stack
  int TOP_val = -1;

  //intermediate value of an expression
  double expVal;

  //final value of a valid expression
  double returnVal;

  //error flag in case of invalidity
  double ERROR_FLAG = -191919.191919;

  //operand stack
  double values[MAXCHAR] = {0};

  //operator stack
  char ops[MAXCHAR] = {0};

  //breaking the string by space and obtaining the tokens
  char * token = strtok(str, " ");

  //equivalent number of the string
  double valC;

  while (token != NULL) {

    //current token is a left parenthesis
    if (token[0] == '(')
      ops[++TOP_ops] = token[0];

    //current token is a right parenthesis
    else if (token[0] == ')') {

      while (TOP_ops > -1 && ops[TOP_ops] != '(') {

        //unary minus calculation
        if (ops[TOP_ops] == '#'){
        
           if(TOP_val > -1) 
             values[TOP_val] = values[TOP_val] * (-1);
             
           else
             return ERROR_FLAG;  
        } 
          
        //insufficient operands
        else if (ops[TOP_ops] != '#' && TOP_val <= 0) 
            return ERROR_FLAG; 
             
        //calculation with binary operator   
        else {
          expVal = expCalculation(values[TOP_val - 1], values[TOP_val], ops[TOP_ops]);

          //invalid operator
          if (expVal == ERROR_FLAG)
            return ERROR_FLAG;

          values[--TOP_val] = expVal;
        }
        --TOP_ops;
      }
      
      //remove left parenthesis from the stack
      if (TOP_ops > -1 && ops[TOP_ops] == '(')
        --TOP_ops;

      else if (TOP_ops == -1 || (TOP_ops > -1 && ops[TOP_ops] != '('))
        return ERROR_FLAG;
    }

    //current token is a number
    else if (isdigit(token[0])) {

      valC = atof(token);
      values[++TOP_val] = valC;
    }

    //current token is an operator
    else {
      while (TOP_ops > -1 && precedence(ops[TOP_ops]) >= precedence(token[0])) {

        // '^' follows right associativity
        if (token[0] == '^' && ops[TOP_ops] == '^') 
          break;

        //unary minus calculation
        else if (ops[TOP_ops] == '#'){
        
           if(TOP_val > -1) 
             values[TOP_val] = values[TOP_val] * (-1);
             
           else
             return ERROR_FLAG;  
        } 
            
        //insufficient operands
        else if (ops[TOP_ops] != '#' && TOP_val <= 0) 
            return ERROR_FLAG;
            
        //calculation with binary operator 
        else {
          expVal = expCalculation(values[TOP_val - 1], values[TOP_val], ops[TOP_ops]);

          //invalid operator
          if (expVal == ERROR_FLAG)
            return ERROR_FLAG;

          values[--TOP_val] = expVal;
        }
        --TOP_ops;
      }

      // Push current token to operator stack.
      ops[++TOP_ops] = token[0];
    }

    token = strtok(NULL, " ");
  }

  //perform calculation from the remaining operators in the stack
  while (TOP_ops > -1) {

     //unary minus calculation
     if (ops[TOP_ops] == '#'){
        
        if(TOP_val > -1) 
          values[TOP_val] = values[TOP_val] * (-1);
             
        else
          return ERROR_FLAG;  
        } 
          
     //insufficient operands
     else if (ops[TOP_ops] != '#' && TOP_val <= 0) 
        return ERROR_FLAG;  
        
     //calculation with binary operator     
     else {
      expVal = expCalculation(values[TOP_val - 1], values[TOP_val], ops[TOP_ops]);

      //invalid operator
      if (expVal == ERROR_FLAG)
        return ERROR_FLAG;

      values[--TOP_val] = expVal;
    }
    --TOP_ops;
  }

  if (TOP_ops == -1 && TOP_val == 0)
    returnVal = values[TOP_val];

  else
    returnVal = ERROR_FLAG;


  return returnVal;
}

/* Conversion of Infix expression into Postfix expression */
int infixToPostfixConversion(char str[], char cli_addr[], int cli_port) {
  char buffer[MAXCHAR] = {0};
  char tokens[MAXCHAR] = {0};
  char ops[MAXCHAR] = {0};
  char storage[MAXCHAR] = {0};
  char tempStore[MAXCHAR] = {0};
  char format_output[MAXCHAR] = {0};
  char str1[2] = {0};
  FILE * fp;
  int len;
  double eval_value;
  int bufLength;
  int i;

  int index = 0;
  int TOP_ops = -1;
  int success = 1;

  //create the text file if does not exist
  fp = fopen("server_records.txt", "a");

  if (fp == NULL) {
    printf("Error!");
    exit(1);
  }

  //error flag in case of invalidity
  double ERROR_FLAG = -191919.191919;

  //removing white spaces from the expression
  for (i = 0; i < strlen(str); i++) {

    //skip a white space
    if (str[i] == ' ' || str[i] == '\t' || str[i] == '\n' || str[i] == '\r')
      continue;

    tokens[index++] = str[i];
  }
  tokens[index] = '\0';

  if (strlen(tokens) == 0)
    return 0;

  index = 0;
  len = strlen(tokens);

  //formatting into a final expression: tokens separated by space
  for (i = 0; i < len; i++) {

    //check1: First index
    if (i == 0) {

      if (tokens[i] == '.') {
        buffer[index++] = '0';
        buffer[index++] = '.';
      } else if (tokens[i] == '-')
        buffer[index++] = '#';

      else
        buffer[index++] = tokens[i];
    }

    //check2: token is a number
    else if (isdigit(tokens[i])) {

      if (isdigit(tokens[i - 1]))
        buffer[index++] = tokens[i];

      else if (tokens[i - 1] == '.')
        buffer[index++] = tokens[i];

      else {
        buffer[index++] = ' ';
        buffer[index++] = tokens[i];
      }
    }

    //check3: token is a decimal point
    else if (tokens[i] == '.') {

      if (isdigit(tokens[i - 1]))
        buffer[index++] = tokens[i];

      else {
        buffer[index++] = ' ';
        buffer[index++] = '0';
        buffer[index++] = '.';
      }
    }

    //check4: token is an operator
    else {

      if (tokens[i - 1] == '(' && tokens[i] == '-') {

        buffer[index++] = ' ';
        buffer[index++] = '#';
      } else {
        buffer[index++] = ' ';
        buffer[index++] = tokens[i];
      }
    }
  }

  buffer[index] = '\0';
  bufLength = strlen(buffer);

  //convert the special character into unary minus
  for (i = 0; i < bufLength; i++) {

    if (buffer[i] == '#')
      tempStore[i] = '-';

    else
      tempStore[i] = buffer[i];
  }

  tempStore[bufLength] = '\0';

  strcpy(storage, buffer);

  if (isValidInfixForm(buffer) != ERROR_FLAG) {

    fprintf(fp, "%s:%d\tInfix Form = %s\t", cli_addr, cli_port, tempStore);
    fflush(fp);
    bzero(tempStore, MAXCHAR);

    //breaking the string by space and obtaining the tokens
    char * token = strtok(storage, " ");

    char output[MAXCHAR] = {0};

    while (token != NULL) {

      //current token is a left parenthesis
      if (token[0] == '(')
        ops[++TOP_ops] = token[0];

      //current token is a right parenthesis
      else if (token[0] == ')') {

        while (TOP_ops > -1 && ops[TOP_ops] != '(') {

            str1[0] = ops[TOP_ops];
            strcat(output, str1);
            strcat(output, " ");
          
            --TOP_ops;
        }

        //remove left parenthesis from the stack
        if (TOP_ops > -1 && ops[TOP_ops] == '(')
          --TOP_ops;
      }

      //current token is a number
      else if (isdigit(token[0])) {

        strcat(output, token);
        strcat(output, " ");
      }

      //current token is an operator
      else {
        while (TOP_ops > -1 && precedence(ops[TOP_ops]) >= precedence(token[0])) {

          //power operator follows right associativity
          if (ops[TOP_ops] == '^')
            break;

          else {
            str1[0] = ops[TOP_ops];
            strcat(output, str1);
            strcat(output, " ");
          }
          TOP_ops--;
        }

        // Push current token to operator stack.
        ops[++TOP_ops] = token[0];
      }

      token = strtok(NULL, " ");
    }

    //pop out the remaining operators from the stack
    while (TOP_ops > -1) {

        str1[0] = ops[TOP_ops];
        strcat(output, str1);
        strcat(output, " ");

        TOP_ops--;
    }

    len = strlen(output);
    output[len - 1] = '\0';
    
    //Format the output string of postfix expression
    for (i = 0; i < strlen(output); i++) {

    if (output[i] == '#')
      format_output[i] = '-';

    else
      format_output[i] = output[i];
     }
    format_output[i] = '\0';

    //File writing
    fprintf(fp, "Postfix Form = %s\t", format_output);
    fflush(fp);

    strcpy(server_reply, format_output);
    strcat(server_reply, " , ");
    
    eval_value = evaluatePostfix(output);
    fprintf(fp, "Value = %.4f\t", eval_value);
    fflush(fp);

    sprintf(tempStore, "%.4f", eval_value);
    strcat(server_reply, tempStore);

  } else
    success = 0;

  return success;
}
