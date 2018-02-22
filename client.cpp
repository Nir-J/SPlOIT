/* Client which sends commands and just prints the reply from server */
#include<string>
#include<sstream>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/sendfile.h>

#include "include/sploit.h"

#include <arpa/inet.h>

#define MAXLEN 100 // Max length of command which we send 

using namespace std;
// Global variables
char server_ip[33];

/* Issues
*
* Need to add get put handling
*/




void* receive_file(void* data){

	istringstream iss((char*)data);

	// Get port information
	string portstring;
	iss >> portstring;  // get
	iss >> portstring; // port
	iss >> portstring; // $PORT

	int port = atoi(portstring.c_str());

	// Getting file size
	string sizestring; 
	iss >> sizestring; // size
	iss >> sizestring; // $SIZE

	// BUG: atoi
	long size = atoi(sizestring.c_str());

	// Connect to the server port
	int recv_socket;
	if( (recv_socket = connect_to_socket(port, server_ip)) == -1){
		fprintf(stderr, "%d and %s\n", port, server_ip);
		fprintf(stderr, "Thread could not connect to server.");
		return NULL;
	}

	// Receiving file
    long remain_data = size;
    // Need to pass in name
    FILE* file = fopen("received_file.txt", "w");
    char buffer[MAXLEN];
    int numbytes;

    while ( (remain_data > 0) && ((numbytes = recv(recv_socket, buffer, MAXLEN-1, 0)) > 0))
    {
            fwrite(buffer, sizeof(char), numbytes, file);
            remain_data -= numbytes;
    }
    fclose(file);
    close(recv_socket);

}

void* send_file( void* data ){

	istringstream iss((char*)data);

	// Get port information
	string portstring;
	iss >> portstring;  // put
	iss >> portstring; // port
	iss >> portstring; // $PORT

	int port = atoi(portstring.c_str());

	// Connect to the server port
	int send_socket;
	if( (send_socket = connect_to_socket(port, server_ip)) == -1){
		fprintf(stderr, "%d and %s\n", port, server_ip);
		fprintf(stderr, "Thread could not connect to server.");
		return NULL;
	}

	// Sending file

	// Need to pass in file name and size
	int file = open("received_file.txt", O_RDONLY);
	struct stat file_stat;   
	stat("received_file.txt", &file_stat);
    
    long remain_data = file_stat.st_size;
    int sent_bytes;


    /* Sending file data */
    while ((remain_data > 0) && (sent_bytes = sendfile(send_socket, file, NULL, MAXLEN-1)))
    {
    	remain_data -= sent_bytes;
    }
    close(file);
    close(send_socket);

}

int main(int argc, char *argv[])
{
	int sockfd, numbytes;  
	char buffer[MAXLEN];
	int port = 0;

	// Command line arguments not provided
	if (argc != 3) {
	    fprintf(stderr,"usage: client server_ip server_port\n");
	    exit(1);
	}

	// If entered port is invalid
	if ((port = atoi(argv[2])) == 0 || port < 1024 || port > 65535){
		fprintf(stderr, "Please enter a valid port number\n");
		exit(1);
	}

	// Connect to server
	if((sockfd = connect_to_socket(port, argv[1])) == -1){
		fprintf(stderr, "Error connecting.\n");
		exit(1);
	}

	// Saving server IP
	strcpy(server_ip, argv[1]);

	printf("Client: connecting to %s\n", argv[1]);

	string previous;
	// Main loop which takes user commands
	while(1){

		// Clearing out buffer
		memset(buffer, 0, MAXLEN);

		// Receiving reply to command / prompt from server
		if ((numbytes = recv(sockfd, buffer, MAXLEN-1, 0)) == -1){
			perror("recv");
			continue;
		}
		// recv returns zero if server closes connection
		if (numbytes == 0){
			printf("Connection closed.\n");
			close(sockfd);
			exit(0);
		}
		buffer[numbytes] = '\0';
		printf("%s",buffer);

		// Check if reply was a get call
		istringstream iss(buffer);
		string reply;
		if (iss >> reply && (reply == "get" || reply == "put")){
			pthread_t thread;
			if(reply == "get"){
				pthread_create(&thread, NULL, receive_file, buffer);
			}
			else{
				pthread_create(&thread, NULL, send_file, buffer);
			}
			
		}

		// Take user input
		if (fgets(buffer, MAXLEN-1, stdin) == NULL){
			fprintf(stderr, "Error reading command");
			close(sockfd);
			exit(1);
		}
		// Saving previous command in case of get / put

		// Sending command to server
		if (send(sockfd, buffer, strlen(buffer), 0) == -1){
			perror("send");
			continue;
		}	
		
	}
	return 0;
}

