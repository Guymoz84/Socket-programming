#include "file_server.h"

fd_set master;
int fdmax;
int checkpoint=0;
int main(int argc, char** argv){
	//defaults
	char* users_file=NULL;
	char* dir_path=NULL;
	char* port=PORT;
	socklen_t size;
	struct sockaddr_in clientaddr;
	int tmp_socket=0;
	int user_num=0;
	int listen_sock=0;
	int check_select=0;
	int i=0;
    fd_set read_fds;
    FD_ZERO(&master);
    FD_ZERO(&read_fds);
	//command line arguments setup
	int check_arg=checkArgumentsServer(argc, argv,&users_file,&dir_path,&port);
	if(check_arg==-1){
		printf("Num of arguments isn't valid\n");
		return -1;
	}
	//server setup
	Server server=setUpUsers(users_file);
	//folders setup
	int check_folder=folderSetUp(&server,dir_path);
	if(check_folder==-1){
		return -1;
	}
	//socket creation
	listen_sock=createSocket();
	//bind
	int check_bind=bindSetUp(listen_sock,port);
	if(check_bind==-1){
		closeAllSockets(fdmax);
		return -1;
	}
	//try to listen
	int check_listen=listen(listen_sock,MAXNUM_CLIENTS);
	if(check_listen==-1){
		printf("Couldn't listen to socket\n");
		closeAllSockets(fdmax);
		return -1;
	}
	size=sizeof(struct sockaddr_in);
    FD_SET(listen_sock,&master);
    fdmax=listen_sock;
	//now connected to client side
	while(1){
		read_fds=master;

		check_select=select(fdmax+1,&read_fds,NULL,NULL,NULL);
		if(check_select==-1){
          	printf("Timeout at select function\n");
        }
        else if(check_select<0){
          	printf("Error on select function: %s\n",strerror(errno));
          	closeAllSockets(fdmax);
          	exit(1);
        }
        for(i=0;i<fdmax+1;++i){
        	if(FD_ISSET(i,&read_fds)){
        		if(i==listen_sock){
        			tmp_socket=accept(listen_sock,(struct sockaddr*)&clientaddr,&size);
        			if(tmp_socket==-1){
        				printf("Couldn't accept socket\n");
        				closeAllSockets(fdmax);
        				return -1;
        			}
        			FD_SET(tmp_socket,&master);
        			if(tmp_socket>fdmax){
        				fdmax=tmp_socket;
        			}
        			send_char(tmp_socket,START);
        			//while((user_num=login(&server,tmp_socket))==-1){
        			send_char(tmp_socket,LOGIN);
        			if((user_num=login(&server,tmp_socket))==-1){
        				if(send_char(tmp_socket,FAIL_LOGIN)==-1){
        					closeSocket(tmp_socket);
        					return -1;
        				}
        			}
        		}
        		else{
        			user_num=-1;
        			for(int j=0;j<server.num_clients;j++){
        				if(i==server.users[j].socket_id){
        					user_num=j;
        					break;
        				}
        			}
        			if(user_num==-1){
        				//login failed
            			if((user_num=login(&server,i))==-1){
            				if(send_char(i,FAIL_LOGIN)==-1){
            					closeSocket(i);
            					return -1;
            				}
            			}
        			}
        			else{
        				serverSession(&server,i,user_num);
        			}

        		}
        	}
        }
	}
	//socket destroy after use
	i=server.num_clients-1;
	while(i>=0){//read from file - user name and password
		if(server.users[i].name!=NULL){
			free(server.users[i].name);
		}
		if(server.users[i].password!=NULL){
			free(server.users[i].password);
		}
		i--;
	}
	closeAllSockets(fdmax);
	return 1;
}

int checkArgumentsServer(int argc,char** argv,char** users_file,char** dir_path,char** port){
	if(argc==3){
        *users_file=argv[1];
        *dir_path=argv[2];
        return 1;
    }
    else if(argc==4){
        *users_file=argv[1];
        *dir_path=argv[2];
        *port=argv[3];
    	return 1;
    }
    else{
        return -1;
    }
}
Server setUpUsers(char* users_file){
	Server server;
	server.num_clients=0;
	int i=0;
	FILE *file=fopen(users_file,"r");
	if(file==NULL){
		printf("couldn't open the users_file\n");
		return server;
	}
	char* name=calloc(MAXLENGTH_NAME,sizeof(char));
	if(name==NULL){
		printf("Can't calloc name\n");
		fclose(file);
		return server;
	}
	char* password=calloc(MAXLENGTH_NAME,sizeof(char));
	if(password==NULL){
		printf("Can't calloc password\n");
		free(name);
		fclose(file);
		return server;
	}
	while(i<MAXNUM_CLIENTS&&fscanf(file,"%s\t%s\n",name,password)==2){//read from file - user name and password
		strcpy(server.users[i].name,name);
		strcpy(server.users[i].password,password);
		server.users[i].num_files=0;
		server.users[i].socket_id=-1;
		server.num_clients++;
		i++;
	}
	return server;
}
int folderSetUp(Server* server,char* dir_path){
	int i=0;
	FILE* file;
	char* path=calloc(MAXLENGTH_PATH,sizeof(char));
	char path_msg[MAXLENGTH_PATH];
	if(path==NULL){
		printf("Can't calloc path\n");
		return -1;
	}
	strcpy(path,dir_path);
	strcpy(server->dir_path,path);
	while(i<server->num_clients){//create folder for each user
		strcat(path,"/");
		strcat(path,server->users[i].name);
		mkdir(path,S_IRWXU);
		strcpy(path,dir_path);

		strcpy(path_msg,server->dir_path);
		strcat(path_msg,"/");
		strcat(path_msg,server->users[i].name);
		strcat(path_msg,"/");
		strcat(path_msg,"Messages_received_offline.txt");
		file=fopen(path_msg,"w");
		if(file==NULL){
			return -1;
		}
		if(fclose(file)<0){
			printf("Fail to close the Messages_received_offline.txt file\n");
			return -1;
		}
		i++;
	}
	free(path);
	return 1;
}
int bindSetUp(int socket_fd,char* port){
	struct sockaddr_in my_addr;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons((uint16_t) strtol(port, NULL, 10));
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(socket_fd,(struct sockaddr*)&(my_addr),sizeof(my_addr))==-1){
    	printf("Can't bind\n");
    	return -1;
    }
	return 1;
}
int login(Server* server,int socket_fd){
	char* num=calloc(5,sizeof(char));
	char name[MAXLENGTH_NAME]={0};
	char password[MAXLENGTH_PASSWORD]={0};
	char buffer[BUFFER_SIZE]={0};
	if(receiveData(socket_fd,name)==-1){
		printf("Failed to receive data from client\n");
		return -1;
	}
	if(receiveData(socket_fd,password)==-1){
		printf("Failed to receive data from client\n");
		return -1;
	}
	for(int i=0;i<server->num_clients;i++){
		if(strcmp(server->users[i].name,name)==0 && strcmp(server->users[i].password,password)==0){
			server->users[i].socket_id=socket_fd;
			send_char(socket_fd,PRINT);
			sprintf(num,"%d",server->users[i].num_files);
			buffer[0]='\0';
			strcat(buffer,"Hi ");
			strcat(buffer,name);
			strcat(buffer,", you have ");
			strcat(buffer,num);
			strcat(buffer," files stored.\n");
			sendData(socket_fd,buffer);
			send_char(socket_fd,YOUR_TURN);
			free(num);
			return i;
		}
	}
	free(num);
	return -1;
}
void serverSession(Server* server,int socket_fd,int user_num){
	char instruction_at_server_session=receive_char(socket_fd);
	switch(instruction_at_server_session){
		case LIST_OF_FILES:
			listOfFiles(*server,socket_fd,user_num);
			break;
		case DELETE_FILE:
			deleteFile(server,socket_fd,user_num);
			break;
		case ADD_FILE:
			addFile(server,socket_fd,user_num);
			break;
		case GET_FILE:
			getFile(*server,socket_fd,user_num);
			break;
		case USERS_ONLINE:
			usersOnline(*server,socket_fd,user_num);
			break;
		case MSG:
			msg(server,socket_fd,user_num);
			break;
		case READ_MSGS:
			readMsgs(*server,socket_fd,user_num);
			break;
		case QUIT:
			quit(server,socket_fd,user_num);
			return;//session ended
		default:
			printf("Instruction is invalid\n");
			break;
	}
}
int listOfFiles(Server server, int socket_fd,int user_num){
	char buffer[BUFFER_SIZE]={0};
	buffer[0]='\0';
	for(int i=0;i<server.users[user_num].num_files;i++){
		send_char(socket_fd,PRINT);
		strcpy(buffer,server.users[user_num].files[i]);
		strcat(buffer,"\n");
		sendData(socket_fd,buffer);
	}
	send_char(socket_fd,YOUR_TURN);
	return 1;
}
int deleteFile(Server* server, int socket_fd,int user_num){
	char file_name[MAXLENGTH_FILE_NAME]={0};
	char path[MAXLENGTH_PATH]={0};
	receiveData(socket_fd,file_name);
	for(int i=0; i<server->users[user_num].num_files;i++){
		if(strcmp(server->users[user_num].files[i],file_name)==0){
			strcpy(path,server->dir_path);
			strcat(path,"/");
			strcat(path,server->users[user_num].name);
			strcat(path,"/");
			strcat(path,file_name);
			remove(path);
			for(int j=i;j<server->users[user_num].num_files-1;j++){
				strcpy(server->users[user_num].files[j],server->users[user_num].files[j+1]);
			}
			strcpy(server->users[user_num].files[server->users[user_num].num_files],"");
			server->users[user_num].num_files--;
			send_char(socket_fd,PRINT);
			sendData(socket_fd,"File removed\n");
			send_char(socket_fd,YOUR_TURN);
			return 1;
		}
	}
	send_char(socket_fd,PRINT);
	sendData(socket_fd,"No such file exists!\n");
	send_char(socket_fd,YOUR_TURN);
	return -1;
}
int addFile(Server* server, int socket_fd,int user_num){
	char buffer[BUFFER_SIZE]={0};
	char name[MAXLENGTH_FILE_NAME]={0};
	char path[MAXLENGTH_PATH]={0};
	FILE* file;
	buffer[0]='\0';
	receiveData(socket_fd,buffer);
	receiveData(socket_fd,name);
	strcpy(path,server->dir_path);
	strcat(path,"/");
	strcat(path,server->users[user_num].name);
	strcat(path,"/");
	name[strlen(name)-1]='\0';
	strcat(path,name);
	file=fopen(path,"w");
	if(file==NULL){
		printf("Can't open the file when adding fine\n");
		return -1;
	}
	fwrite(buffer,sizeof(char),strlen(buffer),file);
	strcpy(server->users[user_num].files[server->users[user_num].num_files],name);
	server->users[user_num].num_files++;
	fclose(file);
	send_char(socket_fd,PRINT);
	sendData(socket_fd,"File added\n");
	send_char(socket_fd,YOUR_TURN);
	return 1;
}
int getFile(Server server, int socket_fd,int user_num){
	char buffer[BUFFER_SIZE]={0};
	char file_name[MAXLENGTH_FILE_NAME]={0};
	char path[MAXLENGTH_PATH]={0};
	FILE* file;
	receiveData(socket_fd,file_name);
	for(int i=0;i<server.users[user_num].num_files;i++){
		if(strcmp(server.users[user_num].files[i],file_name)==0){
			strcpy(path,server.dir_path);
			strcat(path,"/");
			strcat(path,server.users[user_num].name);
			strcat(path,"/");
			strcat(path,file_name);
			file=fopen(path,"r");
			if(file==NULL){
				printf("Can't open the file at get file command\n");
				return -1;
			}
			fread(buffer,sizeof(char),BUFFER_SIZE,file);
			fclose(file);
			sendData(socket_fd,buffer);
			send_char(socket_fd,YOUR_TURN);
			return 1;
		}
	}
	send_char(socket_fd,YOUR_TURN);
	return -1;
}
int usersOnline(Server server, int socket_fd,int user_num){
	char buffer[BUFFER_SIZE]={0};
	int not_first=0;
	buffer[0]='\0';
	send_char(socket_fd,PRINT);
	sendData(socket_fd,"online users: ");
	for(int i=0;i<server.num_clients;i++){
		if(server.users[i].socket_id>0){//user is online
			send_char(socket_fd,PRINT);
			if(not_first){
				strcpy(buffer,", ");
			}
			strcat(buffer,server.users[i].name);
			sendData(socket_fd,buffer);
			not_first=1;
		}
	}
	send_char(socket_fd,PRINT);
	sendData(socket_fd,"\n");
	send_char(socket_fd,YOUR_TURN);
	return 1;
}
int msg(Server* server, int socket_fd,int user_num){
	int i=0;
	int user_num2=-1;
	char buffer[BUFFER_SIZE]={0};
	char message[BUFFER_SIZE]={0};
	char user_name[MAXLENGTH_NAME]={0};
	char path[MAXLENGTH_PATH]={0};
	FILE* file;
	receiveData(socket_fd,user_name);
	receiveData(socket_fd,message);
	sprintf(buffer,"from %s:%s\n",server->users[user_num].name,message);
	for(i=0;i<server->num_clients;i++){
		if(strcmp(server->users[i].name,user_name)==0){
			user_num2=i;
			break;
		}
	}
	if(user_num2==-1){
		printf("Can't find the user\n");
		return -1;
	}
	if(server->users[user_num2].socket_id>0){//user connected
		send_char(server->users[user_num2].socket_id,PRINT);
		sendData(server->users[user_num2].socket_id,"New message ");
		send_char(server->users[user_num2].socket_id,PRINT);
		sendData(server->users[user_num2].socket_id,buffer);
		send_char(server->users[user_num2].socket_id,YOUR_TURN);
	}
	else{//user disconnected
		strcpy(path,server->dir_path);
		strcat(path,"/");
		strcat(path,server->users[user_num2].name);
		strcat(path,"/");
		strcat(path,"Messages_received_offline.txt");
		file=fopen(path,"a");
		if(file==NULL){
			printf("Can't open the file when adding fine\n");
			return -1;
		}
		fwrite("Message received ",sizeof(char),17,file);
		fwrite(buffer,sizeof(char),strlen(buffer),file);
		fclose(file);
	}
	send_char(socket_fd,YOUR_TURN);
	return 1;
}
int readMsgs(Server server, int socket_fd,int user_num){
	char message[101];
	message[100]='\0';
	char path[MAXLENGTH_PATH];
	FILE* file;
	strcpy(path,server.dir_path);
	strcat(path,"/");
	strcat(path,server.users[user_num].name);
	strcat(path,"/");
	strcat(path,"Messages_received_offline.txt");
	file=fopen(path,"r");//need to create path
	if(file==NULL){
		return -1;
	}
	while(fgets(message,101,file)!=NULL){
		send_char(socket_fd,PRINT);
		sendData(socket_fd,message);
	}
	if(fclose(file)<0){
		return -1;
	}
	file=fopen(path,"w");//for cleaning the file
	if(file==NULL){
		return -1;
	}
	if(fclose(file)<0){//for cleaning the file
		return -1;
	}
	send_char(socket_fd,YOUR_TURN);
	return 1;
}
int quit(Server* server, int socket_fd,int user_num){
	server->users[user_num].socket_id=-1;
	closeSocket(socket_fd);
	FD_CLR(socket_fd,&master);
	return 1;
}
