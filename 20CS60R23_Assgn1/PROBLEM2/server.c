/*
THE SERVER PROGRAM

NAME- SREYASREE MANDAL
ROLL NO. 20CS60R23
ASSIGNMENT NO. 1
PROBLEM 2

COMPILE: 
gcc server.c -lm -o server

RUN:
./server <FILE NAME> <PORT NUMBER>

RUN example:
./server input.txt 8080

The command line arguments are as follows:
argv[1] - <FILE NAME>
argv[2] - <PORT NUMBER>

*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <math.h>
#include <ctype.h>
#define MAXCHAR 1024

/* Function declarations */

/* Defines the operator precedence. Higher the value, higher is the precedence */
int precedence(char op);

/*List of operators that can be pushed to the operator stack */
int isOperator(char op); 

/*Resloves the issue of unary minus operator */
int signVal(int valM); 

/*Performs standard mathematical calculation from the list of valid operators */
double calculate(double val1, double val2, char operator);

/* Returns the substring starting at index <start> and ending at <end -1> */
char* substring(char str[], int start, int end);

/*Checks whether two strings are same or not */
int stringCompare(char str1[], char str2[], int len1, int len2);

/*Performs infix evaluation for an expression */
double evaluateInfix(char str[]);

int main(int argc, char * argv[]) {

  int server_fd, new_socket, client_msg;
  struct sockaddr_in address;
  int addrlen = sizeof(address);
  char buffer[1024];
  FILE * fp;
  int PORT;

  int duplicate;
  double ERROR_FLAG = -191919.191919;

  // Creating socket file descriptor 
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  /* Initialize socket structure */
  bzero((char * ) & address, sizeof(address));
  
  PORT = atoi(argv[2]);

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  //Attaching socket to the port 8080 
  if (bind(server_fd, (struct sockaddr * ) & address, sizeof(address)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }
  if (listen(server_fd, 3) < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }
  
  while(1){
  
  //Client connection acceptance
  if ((new_socket = accept(server_fd, (struct sockaddr * ) & address, (socklen_t * ) & addrlen)) < 0) {
    perror("accept");
    exit(EXIT_FAILURE);
  }

  /* If connection is established, begin the communication */
  bzero(buffer, 1024);

  client_msg = read(new_socket, buffer, 1024);
  printf("Message received from the Client: %s\n", buffer);

  //EXpression evaluation query received from the client
  if (buffer[0] == 'E' || buffer[0] == 'e') {

    char * token = strtok(buffer, " ");
    int expNum = atoi(strtok(NULL, " "));

    //open the file in read mode
    fp = fopen(argv[1], "r");

    if (fp == NULL) {
      printf("Could not open files \n");
      return -1;
    }

    //reset the buffer
    bzero(buffer, 1024);
    int flag_line = 0;
    
    int i;
    for (i = 1; fgets(buffer, MAXCHAR, fp) != NULL; i++) {

      if (i == expNum){
        flag_line = 1;
        break;
       } 
    }

    fclose(fp);

    if(!flag_line){
      bzero(buffer, 1024);
      strcpy(buffer, "INVALID LINE NUMBER!!!");
      }

    else{

    //calls function for expression evaluation
    double expValue = evaluateInfix(buffer);
    bzero(buffer, 1024);

    if (expValue == ERROR_FLAG )
      strcpy(buffer, "INVALID EXPRESSION!!!");

    else
      sprintf(buffer, "%.4f", expValue);
    }  

   //Write to the input file query received from the client
  } else if (buffer[0] == 'W' || buffer[0] == 'w') {

    char p1[MAXCHAR], p2[MAXCHAR];

    char * tempS1 = (char * ) malloc(MAXCHAR * sizeof(char));
    char * tempS2 = (char * ) malloc(MAXCHAR * sizeof(char));
    
    int index = 0;

    //breaking the query received from the client into two parts: command and exp
    sscanf(buffer, "%s %[^\r\n]", p1, p2);
    
    //removing white spaces from the expression
    for (int j = 0; j < strlen(p2); j++) {

      //skip a white space  
      if (p2[j] == ' ' || p2[j] == '\t' || p2[j] == '\n' || p2[j] == '\r')
        continue;

      tempS1[index++] = p2[j];

    }
    tempS1[index] = '\0';
    int length1 = strlen(tempS1);
    
    //checking whether the expression sent by the client is valid or not
    if (evaluateInfix(tempS1) == ERROR_FLAG ) {
      bzero(buffer, 1024);
      strcpy(buffer, "INVALID EXPRESSION! It cannot be appended to the file.");
      
    //expression is valid  
    } else {
      //open the file in read mode
      fp = fopen(argv[1], "r");

      if (fp == NULL) {
        printf("Could not open files \n");
        return -1;
      }

      bzero(buffer, 1024);
      
      //opening the file and doing a sequential access to check whether the expression is duplicate or not
      for (int i = 1; fgets(buffer, MAXCHAR, fp) != NULL; i++) {

        index = 0;
        duplicate = 0;
        
        //removing white spaces from the expression
        for (int j = 0; j < strlen(buffer); j++) {

          //skip a white space  
          if (buffer[j] == ' ' || buffer[j] == '\t' || buffer[j] == '\n' || buffer[j] == '\r')
            continue;

          tempS2[index++] = buffer[j];

        }
        tempS2[index] = '\0';
        int length2 = strlen(tempS2);
        
        //checking if the strings are duplicates
        if ( stringCompare(tempS1, tempS2, length1, length2) ) {
          duplicate = 1;
          break;
        }
      }
      fclose(fp);

      if (duplicate) {
        bzero(buffer, 1024);
        strcpy(buffer, "DUPLICATE EXPRESSION! It cannot be appended to the file.");
        
      } else {
      
        //open the file in append mode
        fp = fopen(argv[1], "a");

        /* Write content to the file */
        fprintf(fp, "%s\n", p2);
        fclose(fp);

        bzero(buffer, 1024);
        strcpy(buffer, "SUCCESSFUL!!! Expression appended to the file.");
      }

    }

    free(tempS1);
    free(tempS2);
  }

  //sending appropriate response to the client
  write(new_socket, buffer, sizeof(buffer));
  
  }
  close(new_socket);
  close(server_fd);

  return 0;
}

/* Defines the operator precedence. Higher the value, higher is the precedence */
int precedence(char op) {

  int value = 0;

  if (op == '+' || op == '-')
    value = 1;

  if (op == '*' || op == '/' || op == '%')
    value = 2;

  if (op == '^')
    value = 3;

  return value;
}

/*List of operators that can be pushed to the operator stack */
int isOperator(char op) {

  int value = 0;
  if (op == '+' || op == '-' || op == '*' || op == '/' || op == '%' || op == '^' || op == '(')
    value = 1;

  return value;

}

/*Resloves the issue of unary minus operator */
int signVal(int valM) {

  int sign = 1;

  if (valM % 2 != 0)
    sign = -1;

  return sign;

}

/*Performs standard mathematical calculation from the list of valid operators */
double calculate(double val1, double val2, char operator) {

  //error flag
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
    printf("\n");

  }

  return value;
}

/* Returns the substring starting at index <start> and ending at <end -1> */
char* substring(char str[], int start, int end)
{
    //Length of the substring
    int len = end - start;
 
    //Dynamically allocate memory for the destination string 
    char *dest_str = (char*)malloc(sizeof(char) * (len + 1));
 
    //Get the required destination substring
    for (int i = start; i < end && (*(str + i) != '\0'); i++)
    {
        *dest_str = *(str + i);
        dest_str++;
    }
 
    *dest_str = '\0';
 
    // return the required substring
    return dest_str - len;
}

/*Checks whether two strings are same or not */
int stringCompare(char str1[], char str2[], int len1, int len2){

  //duplicate flag is set to 1 if two strings are found equal  
  int duplicate = 0;
  
  //checking if two strings are equal or not
  if ( strcmp(str1, str2) == 0 )
     duplicate = 1;
     
  //checks if two strings are equal or not after removing corner parenthesis from the first string   
  else if (len1 > len2){
     for(int j = 0; j < len1; j++){
     
        if ( str1[j] == '('  && str1[len1 - j - 1] == ')' ){
           if ( strcmp(str2, substring(str1, j+1, len1 - j - 1) ) == 0) {
              duplicate = 1;
              break;
              }
           } 
           
         else
           break;    
     }
  
  } 
  
  //checks if two strings are equal or not after removing corner parenthesis from the second string
  else if (len2 > len1){
     for(int j = 0; j < len2; j++){
     
        if ( str2[j] == '('  && str2[len2 - j - 1] == ')' ){
           if ( strcmp(str1, substring(str2, j+1, len2 - j - 1)) == 0 ){
              duplicate = 1;
              break;
              }
           }   
           
         else
            break;  
     }
  
  }

  return duplicate;
}

/*Performs infix evaluation for an expression */
double evaluateInfix(char str[]) {

  //TOP of operator stack
  int TOP_ops = -1;

  //TOP of value stack
  int TOP_val = -1;

  //unary minus
  int unaryM_flag = 0;
   
  //intermediate value of an expression
  double expVal;
  
  //final value of a valid expression
  double returnVal;
  
  //error flag in case of invalidity
  double ERROR_FLAG = -191919.191919;
  
  //temporary storage
  char buffer[1024];
  char tokens[1024];
  
  int index = 0;
  int len = strlen(str);

  //removing white or extra spaces from the expression
  for (int j = 0; j < len; j++) {

    //skip a white space  
    if (str[j] == ' ' || str[j] == '\n' || str[j] == '\r' || str[j] == '\t')
      continue;

    buffer[index++] = str[j];

  }
  buffer[index] = '\0';
  len = strlen(buffer);

  if (len == 0)
    return ERROR_FLAG; 

  //resetting
  bzero(tokens, 1024);
  index = 0;

  //handling unary operators from the expression
  for (int j = 0; j < len; j++) {

    if (buffer[j] == '+' && j == 0)
      continue;

    if (buffer[j] == '+' && isOperator(buffer[j - 1]))
      continue;

    if (buffer[j] == '-' && isOperator(buffer[j - 1])) {
      unaryM_flag++;
      continue;
    }

    int sign = signVal(unaryM_flag);

    if (sign == -1) {
      tokens[index++] = '-';
      unaryM_flag = 0;
    }
    tokens[index++] = buffer[j];

  }
  tokens[index] = '\0';

  //resetting buffer
  bzero(buffer, 1024);
  index = 0;
  len = strlen(tokens);

  //formatting into a final expression: tokens separated by space
  for (int j = 0; j < len; j++) {

    //check1: First index
    if (j == 0) {

      if (tokens[j] == '.') {
        buffer[index++] = '0';
        buffer[index++] = '.';
      } else
        buffer[index++] = tokens[j];
    }

    //check2: token is an operator
    else if (isOperator(tokens[j]) || tokens[j] == ')') {
      buffer[index++] = ' ';
      buffer[index++] = tokens[j];
    }

    //check3: token is a number  
    else if (isdigit(tokens[j])) {

      if (tokens[j - 1] == '-' && j == 1)
        buffer[index++] = tokens[j];

      else if (isdigit(tokens[j - 1]))
        buffer[index++] = tokens[j];

      else if (tokens[j - 1] == '.')
        buffer[index++] = tokens[j];

      else if (tokens[j - 1] == '-' && isOperator(tokens[j - 2]))
        buffer[index++] = tokens[j];

      else {
        buffer[index++] = ' ';
        buffer[index++] = tokens[j];
      }

    }

    //check4: token is a decimal point 
    else if (tokens[j] == '.') {

      if (isdigit(tokens[j - 1]))
        buffer[index++] = tokens[j];

      else if (tokens[j - 1] == '-' && isOperator(tokens[j - 2])) {
        buffer[index++] = '0';
        buffer[index++] = '.';
      } else {
        buffer[index++] = ' ';
        buffer[index++] = '0';
        buffer[index++] = '.';
      }
    }
  }

  buffer[index] = '\0';
  
  printf("\nTHE EXPRESSION IS : %s \n", buffer);

  //operand stack
  double * values = (double * ) malloc(len * sizeof(double));

  //operator stack
  char * ops = (char * ) malloc(len * sizeof(char));

  len = strlen(buffer);
  
  //breaking the string by space and obtaining the tokens
  char * token = strtok(buffer, " ");

  while (token != NULL) {

    //current token is a left parenthesis  
    if (token[0] == '(')
      ops[++TOP_ops] = token[0];

    //current token is a right parenthesis
    else if (token[0] == ')') {

      while (TOP_ops > -1 && ops[TOP_ops] != '(') {

        if ( TOP_val <= 0)
           expVal == ERROR_FLAG;
           
        else    
           expVal = calculate(values[TOP_val - 1], values[TOP_val], ops[TOP_ops]);
        
        if ( expVal == ERROR_FLAG )
           return ERROR_FLAG;
           
        values[--TOP_val] = expVal;
        --TOP_ops;
      }

      //remove left parenthesis from the stack
      if (TOP_ops > -1 && ops[TOP_ops] == '(')
        --TOP_ops;

    }

    //current token is a number
    else if (isdigit(token[0]) || (token[0] == '-' && isdigit(token[1]))) {

      double valC = atof(token);
      values[++TOP_val] = valC;
    }

    //current token is an operator
    else {
      while (TOP_ops > -1 && precedence(ops[TOP_ops]) >= precedence(token[0])) {

        // '^' follows right associativity
        if (token[0] == '^' && ops[TOP_ops] == '^') {
          break;
        }

        if ( TOP_val <= 0)
           expVal == ERROR_FLAG;
           
        else   
           expVal = calculate(values[TOP_val - 1], values[TOP_val], ops[TOP_ops]);
        
        if ( expVal == ERROR_FLAG)
           return ERROR_FLAG;
           
        values[--TOP_val] = expVal;
        --TOP_ops;
      }

      // Push current token to operator stack.
      ops[++TOP_ops] = token[0];
    }

    token = strtok(NULL, " ");

  }

  //perform calculation from the remaining operators in the stack
  while (TOP_ops > -1) {

    if ( TOP_val <= 0)
        expVal == ERROR_FLAG;
           
    else   
       expVal = calculate(values[TOP_val - 1], values[TOP_val], ops[TOP_ops]);
    
    if ( expVal == ERROR_FLAG)
           return ERROR_FLAG;
           
    values[--TOP_val] = expVal;
    --TOP_ops;
  }


  if (TOP_ops == -1 && TOP_val == 0)
     returnVal = values[TOP_val];

  else
    returnVal = ERROR_FLAG;

  return returnVal;

}
