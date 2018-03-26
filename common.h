#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <stdbool.h>

#ifndef COMMON_H_
#define COMMON_H_


#define HOSTNAME "localhost"
#define PORT "1337"
#define MAXLENGTH_FILE 512
#define MAXLENGTH_PASSWORD 26
#define MAXLENGTH_NAME 26
#define MAXNUM_FILES 15
#define MAXNUM_CLIENTS 15
#define MAXLENGTH_PATH 500
#define BUFFER_SIZE 1024
#define MAXLENGTH_LINE 256
#define MAXLENGTH_FILE_NAME 50

#define START 'S'
#define LOGIN 'L'
#define WELCOME 'W'
#define YOUR_TURN 'T'
#define FAIL_LOGIN 'F'
#define PRINT 'P'
#define LIST_OF_FILES 'l'
#define DELETE_FILE 'D'
#define ADD_FILE 'A'
#define GET_FILE 'G'
#define USERS_ONLINE 'U'
#define MSG 'M'
#define READ_MSGS 'R'
#define QUIT 'Q'


int createSocket();
int destroySocket(int socket);
int closeSocket(int socket);
int send_char(int socket_fd,char instruction);
int sendAll(int socket_fd,char* buffer,int* len);
int sendData(int socket_fd,char* data);
char receive_char(int socket_fd);
int recvAll(int socket_fd,char* buffer,int* len);
int receiveData(int socket_fd,char* buffer);
int closeAllSockets(int fd);


#endif /* COMMON_H_ */
