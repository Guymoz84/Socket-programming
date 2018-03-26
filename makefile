all: file_client file_server

file_client: file_client.o common.o
	gcc -o file_client file_client.o common.o -Wall -g

file_client.o: file_client.c common.h
	gcc -c -Wall -g file_client.c

file_server: file_server.o common.o
	gcc -o file_server file_server.o common.o -Wall -g

file_server.o: file_server.c common.h
	gcc -c -Wall -g file_server.c

common.o: common.c common.h
	gcc -c -Wall -g common.c

clean:
	-rm file_client.o file_server.o common.o file_client file_server
