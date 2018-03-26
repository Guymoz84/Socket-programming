#include "common.h"



#ifndef FILE_CLIENT_H_
#define FILE_CLIENT_H_


int checkArgumentsClient(int argc,char** argv,char **hostname,char** port);
int connectSocket(int socket_fd,struct addrinfo* server_info);
struct addrinfo* getAddr(char* hostname,char* port);
int start(int socket_fd,char* name,char* password);
void printData(int socket_fd);
int parserClient(int socket_fd);
void sendChatMessage(int sockfd, char *buf);//new function
#endif /* FILE_CLIENT_H_ */
