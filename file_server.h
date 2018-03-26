#include "common.h"

#ifndef FILE_SERVER_H_
#define FILE_SERVER_H_
typedef struct user_t{
	char password[MAXLENGTH_PASSWORD];
	char name[MAXLENGTH_NAME];
	char files[MAXNUM_FILES][MAXLENGTH_FILE_NAME];
	int num_files;
	int socket_id;
}User;
typedef struct server_t{
	User users[MAXNUM_CLIENTS];
	int num_clients;
	char dir_path[MAXLENGTH_PATH];
}Server;
int checkArgumentsServer(int argc,char** argv,char** users_file,char** dir_path,char** port);
Server setUpUsers(char* users_file);
int folderSetUp(Server* server,char* dir_path);
int bindSetUp(int socket,char* port);
int login(Server* server,int socket_fd);
void serverSession(Server* server,int socket_fd,int user_num);
int listOfFiles(Server server, int socket_fd,int user_num);
int deleteFile(Server* server, int socket_fd,int user_num);
int addFile(Server* server, int socket_fd,int user_num);
int getFile(Server server, int socket_fd,int user_num);
int usersOnline(Server server, int socket_fd,int user_num);
int msg(Server* server, int socket_fd,int user_num);
int readMsgs(Server server, int socket_fd,int user_num);
int quit(Server* server, int socket_fd,int user_num);
#endif /* FILE_SERVER_H_ */
