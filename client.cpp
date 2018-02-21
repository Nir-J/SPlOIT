/* Client which sends commands and just prints the reply from server */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "include/sploit.h"

#include <arpa/inet.h>

#define MAXLEN 100 // Max length of command which we send 

/* Issues
*
* Need to add get put handling
*/


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

	printf("Client: connecting to %s\n", argv[1]);

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

		// Take user input
		if (fgets(buffer, MAXLEN-1, stdin) == NULL){
			fprintf(stderr, "Error reading command");
			close(sockfd);
			exit(1);
		}
		// Sending command to server
		if (send(sockfd, buffer, strlen(buffer), 0) == -1){
			perror("send");
			continue;
		}	
		
	}
	return 0;
}

