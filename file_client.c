#include "file_client.h"

#define STDIN 0
fd_set read_fds;
int checkpoint=0;

int main(int argc, char** argv){
	//defaults
	char* hostname=HOSTNAME;
	char* port = PORT;
	char name[MAXLENGTH_NAME]={0};
	char password[MAXLENGTH_PASSWORD]={0};
	int check_select=0;
	//command line arguments setup
	int check_arg=checkArgumentsClient(argc, argv, &hostname, &port);
	if(check_arg==-1){
		printf("Num of arguments isn't valid\n");
		return -1;
	}
	//socket creation
	int socket_fd=createSocket();
	//get address info
	struct addrinfo* server_info=getAddr(hostname,port);
	//connect
	int check_connect=connectSocket(socket_fd,server_info);
	if(check_connect==-1){
		closeSocket(socket_fd);
		return -1;
	}
	//free server address (don't need it anymore)
	freeaddrinfo(server_info);
	//connected to the server
	while(1){
		char instruction=receive_char(socket_fd);
		switch(instruction){
			case START:
				printf("Welcome! Please log in.\n");
				break;
			case LOGIN:
				while(start(socket_fd,name,password)==-1);
				break;
			case FAIL_LOGIN:
				printf("Login failed, try again\n");
				while(start(socket_fd,name,password)==-1);
				break;
			case PRINT:
				printData(socket_fd);
				break;
			case YOUR_TURN:
                FD_ZERO(&read_fds);
                FD_SET(STDIN,&read_fds);
                FD_SET(socket_fd,&read_fds);
                check_select=select(socket_fd+1,&read_fds,NULL,NULL,NULL);
        		if(check_select==-1){
                  	printf("Timeout at select function\n");
                }
                else if(check_select<0){
                  	printf("Error on select function: %s\n",strerror(errno));
                  	closeSocket(socket_fd);
                  	exit(1);
                }
                if (FD_ISSET(STDIN,&read_fds)){
                	if(parserClient(socket_fd)==-1){//==-1 iff quit
                		//socket destroy after use
                		closeSocket(socket_fd);
                		return 1;
                	}
                }
				break;
			default: break;
		}
	}
	return 1;
}
int checkArgumentsClient(int argc,char** argv,char** hostname,char** port){
    if (argc==1){
        return 1;
    }
    else if(argc==2){
        *hostname = argv[1];
        return 1;
    }
    else if(argc==3){
    	*hostname=argv[1];
    	*port=argv[2];
    	return 1;
    }
    else{
        return -1;
    }
}
int connectSocket(int socket_fd,struct addrinfo* server_info){
	int check=0;
	struct addrinfo* current=server_info;
	while(current!=NULL){
		check=connect(socket_fd,current->ai_addr,current->ai_addrlen);
		if(check==0){
			return 1;
		}
		current=current->ai_next;
	}
	printf("Couldn't connect to server\n");
	return -1;
}
struct addrinfo* getAddr(char* hostname,char* port){
	struct addrinfo hints;
	struct addrinfo* servinfo;
	memset(&hints,0,sizeof(hints));
	//hints.ai_family=AF_UNSPEC;
	hints.ai_family=AF_INET;
	hints.ai_socktype=SOCK_STREAM;
	int check_addr=getaddrinfo(hostname,port,&hints,&servinfo);
	if(check_addr==-1){
		printf("Couldn't get the address info\n");
		return NULL;
	}
	return servinfo;
}
int start(int socket_fd,char* name,char* password){
	char* buffer=calloc(MAXLENGTH_NAME+9,sizeof(char));
	if(buffer==NULL){
		printf("Can't calloc for buffer in stat function\n");
		return -1;
	}
	char* token=NULL;
	if(fgets(buffer,MAXLENGTH_NAME+7,stdin)==NULL){
		printf("Input as user isn't valid\n");
		free(buffer);
		return -1;
	}
	token=strtok(buffer," ");
	if(strcmp(token,"User:")!=0){
		printf("Input as user isn't valid\n");
		free(buffer);
		return -1;
	}
	token=strtok(NULL,"\n");
	if(strlen(token)>MAXLENGTH_NAME){
		printf("Input as user isn't valid\n");
		free(buffer);
		return -1;
	}
	strcpy(name,token);
	if(fgets(buffer,MAXLENGTH_PASSWORD+9,stdin)==NULL){
		printf("Input as password isn't valid\n");
		free(buffer);
		return -1;
	}
	token=strtok(buffer," ");
	if(strcmp(token,"Password:")!=0){
		printf("Input as password isn't valid\n");
		free(buffer);
		return -1;
	}
	token=strtok(NULL,"\n");
	if(strlen(token)>MAXLENGTH_PASSWORD){
		printf("Input as password isn't valid\n");
		free(buffer);
		return -1;
	}
	strcpy(password,token);
	free(buffer);
	if(sendData(socket_fd,name)==-1){
		printf("Fail to send name to server\n");
		return -1;
	}
	if(sendData(socket_fd,password)==-1){
		printf("Fail to send password to server\n");
		return -1;
	}
	return 1;
}
void printData(int socket_fd){
	char buffer[BUFFER_SIZE]={0};
	receiveData(socket_fd,buffer);
	printf("%s",buffer);
}
int parserClient(int socket_fd){
	char buffer[BUFFER_SIZE]={0};
	char file_buffer[BUFFER_SIZE]={0};
	char file_path[MAXLENGTH_PATH]={0};
	char file_name[MAXLENGTH_FILE_NAME]={0};
	char* token=NULL;
	FILE* file=NULL;
	while(fgets(buffer,MAXLENGTH_LINE,stdin)!=NULL){
		token=strtok(buffer," \n");
		if(strcmp(token,"list_of_files")==0){
			if(send_char(socket_fd,LIST_OF_FILES)==-1){
				return -1;
			}
			return 1;
		}
		else if(strcmp(token,"delete_file")==0){
			if(send_char(socket_fd,DELETE_FILE)==-1){
				return -1;
			}
			token=strtok(NULL," ");
			token[strlen(token)-1]='\0';
			sendData(socket_fd,token);
			return 1;
		}
		else if(strcmp(token,"add_file")==0){
			if(send_char(socket_fd,ADD_FILE)==-1){
				return -1;
			}
			//the file path
			token=strtok(NULL," ");
			file=fopen(token,"r");
			if(file==NULL){
				printf("Failed to open the file at client\n");
				return -1;
			}
			fread(file_buffer,sizeof(char),BUFFER_SIZE,file);
			file_buffer[BUFFER_SIZE-1]='\0';
			sendData(socket_fd,file_buffer);
			//the new name
			token=strtok(NULL," ");
			sendData(socket_fd,token);
			fclose(file);
			return 1;
		}
		else if(strcmp(token,"get_file")==0){
			if(send_char(socket_fd,GET_FILE)==-1){
				return -1;
			}
			//first token is the file name
			token=strtok(NULL," ");
			strcpy(file_name,token);
			//sending the name to the server
			sendData(socket_fd,token);
			receiveData(socket_fd,file_buffer);
			//second token is the path to save the file
			token=strtok(NULL," ");
			token[strlen(token)-1]='\0';
			strcpy(file_path,token);
			strcat(file_path,"/");
			strcat(file_path,file_name);
			//opening the file to write
			file=fopen(file_path,"w");
			//copying the data into the file
			fwrite(file_buffer,sizeof(char),strlen(file_buffer),file);
			fclose(file);
			return 1;
		}
	    else if(strcmp(token,"users_online")==0){
			if(send_char(socket_fd,USERS_ONLINE)==-1){
				return -1;
			}
	    	return 1;
	    }
	    else if(strcmp(token,"msg")==0){
			if(send_char(socket_fd,MSG)==-1){
				return -1;
			}
			//the user name we want to send
			token=strtok(NULL,": ");//token has the user name
			sendData(socket_fd,token);
			token=strtok(NULL,"\n");
			token[strlen(token)]='\0';//token has the message to send
			sendData(socket_fd,token);
			return 1;
	    }
	    else if(strcmp(token,"read_msgs")==0){
			if(send_char(socket_fd,READ_MSGS)==-1){
				return -1;
			}
			return 1;
	    }
		else if(strcmp(token,"quit")==0){
			send_char(socket_fd,QUIT);
			return -1;
		}
		else{
			printf("Invalid command!\nPlease enter a valid command\n");
		}
	}
	return 1;
}
