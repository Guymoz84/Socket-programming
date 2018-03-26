#include "common.h"
int createSocket(){
	int socket_fd=-1;
	while((socket_fd=socket(PF_INET,SOCK_STREAM,0))==-1){
		printf("Failed to create a new socket\n");
	}
	return socket_fd;
}
/*int destroySocket(int socket_fd){
    int res=0;
    char buffer[BUFFER_SIZE];
    shutdown(socket_fd, SHUT_WR);
    while(1){
         read until the end of the stream
        res = recv(socket_fd, buffer, 10, 0);
        if(res<0){
            printf("Can't shut down the socket\n");
        }
        else if(res==0){
        	break;
        }
    }
    if(closeSocket(socket_fd)==1){
    	return 1;
    }
    return -1;
}*/
int closeSocket(int socket_fd){
	if(socket_fd==-1)
		return -1;
	if(close(socket_fd)==-1){
		perror("Can't close a socket\n");
		return -1;
	}
	return 1;
}
int send_char(int socket_fd,char instruction){
	char send_instruction=instruction;
	if(send(socket_fd,&send_instruction,1,0)==-1){
		perror("Fail to send instruction\n");
		return -1;
	}
	return 1;
}
int sendAll(int socket_fd,char* buffer, int* len){//taken from class
	int total=0;
	int bytesleft=*len;
	int n=0;
    while(total<*len){
        n=send(socket_fd,buffer+total,bytesleft,0);
        if(n==-1){
            break;
        }
        total+=n;
        bytesleft-=n;
    }
    *len=total; /* return number actually sent here */
    if(n==-1){
		perror("Fail to send instruction\n");
		closeSocket(socket_fd);
		free(buffer);
    	return -1;
    }
	return 1;
}
int sendData(int socket_fd,char* data){
	short dataSize = (short)strlen(data);
    short dataSize_networkOrder = htons((uint16_t) dataSize);
    int len=sizeof(dataSize_networkOrder);
    if(sendAll(socket_fd, (char*)&dataSize_networkOrder,&len)==-1){
    	printf("All the data hasn't been send - first send\n");
    	return -1;
    }
    len=(int)dataSize;
    if(sendAll(socket_fd,data, &len)==-1){
    	printf("All the data hasn't been send- second send\n");
    	return -1;
    }
	return 1;
}
char receive_char(int socket_fd){
	char instruction;
	recv(socket_fd,&instruction,1,0);
	return instruction;
}
int recvAll(int socket_fd,char* buffer,int* len){
    int total = 0; /* how many bytes we've read */
    int bytesleft = *len; /* how many we have left to read */
    int n = 0;
    while (total<*len){
        n=recv(socket_fd,buffer+total,bytesleft,0);
        if(n<0){
            perror("Failed receiving data");
            closeSocket(socket_fd);
            return -1;
        }
        if(n==0)/* means the sender has closed the connection */
        {
            *len = 0;
            break;
        }
        total += n;
        bytesleft -= n;
    }
    *len = total; /* return number actually sent here */
    return n;
}
int receiveData(int socket_fd,char* buffer){
    short dataSize;
    int len=sizeof(dataSize);
    if (recvAll(socket_fd,(char*)&dataSize,&len)==0){
        return -1;
    }
    /* set to host bytes order */
    dataSize = ntohs((uint16_t) dataSize);
    len=(int)dataSize;
    int n = recvAll(socket_fd, buffer, &len);
    buffer[dataSize] = '\0'; /* null terminator to terminate the string */
    if(len==n){
    	return 1;
    }
    return -1;
}
int closeAllSockets(int fd){
	for(int i=0;i<fd+1;i++){
		if(closeSocket(i)==-1){
			return -1;
		}
	}
    shutdown(fd,SHUT_WR);
	return 1;
}
